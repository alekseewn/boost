
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#include "boost/fiber/algo/work_stealing.hpp"

#include <random>

#include <boost/assert.hpp>
#include <boost/context/detail/prefetch.hpp>
#include "boost/fiber/detail/thread_barrier.hpp"
#include "boost/fiber/type.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace algo {

std::atomic< std::uint32_t > work_stealing::counter_{ 0 };
std::vector< intrusive_ptr< work_stealing > > work_stealing::schedulers_{};

#if defined(__linux__) && defined(BOOST_FIBERS_USE_LIBURING)
thread_local work_stealing * work_stealing::current_{ nullptr };
#endif

void
work_stealing::init_( std::uint32_t thread_count,
                      std::vector< intrusive_ptr< work_stealing > > & schedulers) {
    // resize array of schedulers to thread_count, initilized with nullptr
    std::vector< intrusive_ptr< work_stealing > >{ thread_count, nullptr }.swap( schedulers);
}

work_stealing::work_stealing( std::uint32_t thread_count, bool suspend) :
        id_{ counter_++ },
        thread_count_{ thread_count },
        suspend_{ suspend } {
    static boost::fibers::detail::thread_barrier b{ thread_count };
    // initialize the array of schedulers
    static std::once_flag flag;
    std::call_once( flag, & work_stealing::init_, thread_count_, std::ref( schedulers_) );
    // register pointer of this scheduler
    schedulers_[id_] = this;
    b.wait();

#if defined(__linux__) && defined(BOOST_FIBERS_USE_LIBURING)
    ring_entries_ = 1024;
    int ret = io_uring_queue_init(ring_entries_, &ring_, 0);
    if (ret < 0) {
        std::abort();
    }
    current_ = this;
#endif
}

void
work_stealing::awakened( context * ctx) noexcept {
    if ( ! ctx->is_context( type::pinned_context) ) {
        ctx->detach();
    }
    rqueue_.push( ctx);
}

context *
work_stealing::pick_next() noexcept {
    context * victim = rqueue_.pop();
    if ( nullptr != victim) {
        boost::context::detail::prefetch_range( victim, sizeof( context) );
        if ( ! victim->is_context( type::pinned_context) ) {
            context::active()->attach( victim);
        }
    } else if (schedulers_.size() > 1) {
        std::uint32_t id = 0;
        std::size_t count = 0, size = schedulers_.size();
        static thread_local std::minstd_rand generator{ std::random_device{}() };
        std::uniform_int_distribution< std::uint32_t > distribution{
            0, static_cast< std::uint32_t >( thread_count_ - 1) };
        do {
            do {
                ++count;
                // random selection of one logical cpu
                // that belongs to the local NUMA node
                id = distribution( generator);
                // prevent stealing from own scheduler
            } while ( id == id_);
            // steal context from other scheduler
            victim = schedulers_[id]->steal();
        } while ( nullptr == victim && count < size);
        if ( nullptr != victim) {
            boost::context::detail::prefetch_range( victim, sizeof( context) );
            BOOST_ASSERT( ! victim->is_context( type::pinned_context) );
            context::active()->attach( victim);
        }
    }
    return victim;
}

void
work_stealing::suspend_until( std::chrono::steady_clock::time_point const& time_point) noexcept {
#if defined(__linux__) && defined(BOOST_FIBERS_USE_LIBURING)
    io_uring_submit(&ring_);
    process_cqes_();
    if (has_ready_fibers()) return;

    if ( suspend_) {
        struct io_uring_cqe * cqe = nullptr;
        if ((std::chrono::steady_clock::time_point::max)() == time_point) {
            io_uring_wait_cqe(&ring_, &cqe);
        } else {
            auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(
                time_point.time_since_epoch()).count();
            __kernel_timespec ts{
                static_cast<__kernel_time_t>(nsec / 1000000000ULL),
                static_cast<__kernel_long_t>(nsec % 1000000000ULL)
            };
            io_uring_wait_cqe_timeout(&ring_, &cqe, &ts);
        }
        if (cqe) process_cqes_();
    }
#else
    if ( suspend_) {
        if ( (std::chrono::steady_clock::time_point::max)() == time_point) {
            std::unique_lock< std::mutex > lk{ mtx_ };
            cnd_.wait( lk, [this](){ return flag_; });
            flag_ = false;
        } else {
            std::unique_lock< std::mutex > lk{ mtx_ };
            cnd_.wait_until( lk, time_point, [this](){ return flag_; });
            flag_ = false;
        }
    }
#endif
}

void
work_stealing::notify() noexcept {
#if defined(__linux__) && defined(BOOST_FIBERS_USE_LIBURING)
    io_uring_sqe * sqe = io_uring_get_sqe(&ring_);
    if (sqe) {
        io_uring_prep_nop(sqe);
        sqe->user_data = 0;
    }
    io_uring_submit(&ring_);
#else
    if ( suspend_) {
        std::unique_lock< std::mutex > lk{ mtx_ };
        flag_ = true;
        lk.unlock();
        cnd_.notify_all();
    }
#endif
}

#if defined(__linux__) && defined(BOOST_FIBERS_USE_LIBURING)
void
work_stealing::process_cqes_() noexcept {
    unsigned head;
    unsigned count = 0;
    struct io_uring_cqe * cqe;

    io_uring_for_each_cqe(&ring_, head, cqe) {
        ++count;
        if (cqe->user_data == 0) continue;

        auto * op = reinterpret_cast<iouring_op *>(
            static_cast<uintptr_t>(cqe->user_data));
        op->result = cqe->res;
        awakened(op->ctx);
    }

    if (count > 0) {
        io_uring_cq_advance(&ring_, count);
    }
}
#endif

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

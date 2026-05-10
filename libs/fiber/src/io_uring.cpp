
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/fiber/algo/work_stealing.hpp>
#include <boost/fiber/io_uring.hpp>
#include <boost/fiber/context.hpp>
#include <boost/assert.hpp>

#include <liburing.h>

namespace boost {
namespace fibers {
namespace io_uring {

int read(int fd, void * buf, unsigned nbytes, uint64_t offset) {
    algo::work_stealing * algo = algo::work_stealing::current();
    BOOST_ASSERT_MSG(algo, "work_stealing not active on this thread");

    algo::work_stealing::iouring_op op;
    op.ctx = context::active();
    op.result = 0;

    io_uring_sqe * sqe = io_uring_get_sqe(&algo->get_ring());
    BOOST_ASSERT_MSG(sqe, "io_uring SQ full");
    io_uring_prep_read(sqe, fd, buf, nbytes, offset);
    sqe->user_data = reinterpret_cast<__u64>(&op);

    io_uring_submit(&algo->get_ring());
    context::active()->suspend();

    return op.result;
}

int write(int fd, void const * buf, unsigned nbytes, uint64_t offset) {
    algo::work_stealing * algo = algo::work_stealing::current();
    BOOST_ASSERT_MSG(algo, "work_stealing not active on this thread");

    algo::work_stealing::iouring_op op;
    op.ctx = context::active();
    op.result = 0;

    io_uring_sqe * sqe = io_uring_get_sqe(&algo->get_ring());
    BOOST_ASSERT_MSG(sqe, "io_uring SQ full");

    io_uring_prep_write(sqe, fd, buf, nbytes, offset);
    sqe->user_data = reinterpret_cast<__u64>(&op);

    io_uring_submit(&algo->get_ring());
    context::active()->suspend();

    return op.result;
}

int pread(int fd, void * buf, unsigned nbytes, uint64_t offset) {
    algo::work_stealing * algo = algo::work_stealing::current();
    BOOST_ASSERT_MSG(algo, "work_stealing not active on this thread");

    algo::work_stealing::iouring_op op;
    op.ctx = context::active();
    op.result = 0;

    io_uring_sqe * sqe = io_uring_get_sqe(&algo->get_ring());
    BOOST_ASSERT_MSG(sqe, "io_uring SQ full");
    io_uring_prep_read(sqe, fd, buf, nbytes, offset);
    sqe->user_data = reinterpret_cast<__u64>(&op);

    io_uring_submit(&algo->get_ring());
    context::active()->suspend();

    return op.result;
}

int pwrite(int fd, void const * buf, unsigned nbytes, uint64_t offset) {
    algo::work_stealing * algo = algo::work_stealing::current();
    BOOST_ASSERT_MSG(algo, "work_stealing not active on this thread");

    algo::work_stealing::iouring_op op;
    op.ctx = context::active();
    op.result = 0;

    io_uring_sqe * sqe = io_uring_get_sqe(&algo->get_ring());
    BOOST_ASSERT_MSG(sqe, "io_uring SQ full");
    io_uring_prep_write(sqe, fd, buf, nbytes, offset);
    sqe->user_data = reinterpret_cast<__u64>(&op);

    io_uring_submit(&algo->get_ring());
    context::active()->suspend();

    return op.result;
}

}}}

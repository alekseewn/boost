
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_IO_URING_H
#define BOOST_FIBERS_IO_URING_H

#if defined(__linux__) && defined(BOOST_FIBERS_USE_LIBURING)

#include <boost/assert.hpp>

#include <boost/fiber/algo/work_stealing.hpp>
#include <boost/fiber/context.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace io_uring {

inline
int read(int fd, void * buf, unsigned nbytes, __u64 offset = 0) {
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

inline
int write(int fd, void const * buf, unsigned nbytes, __u64 offset = 0) {
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

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif
#endif

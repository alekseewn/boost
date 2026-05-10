
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_IO_URING_H
#define BOOST_FIBERS_IO_URING_H

#include <cstdint>

#include <boost/fiber/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace io_uring {

BOOST_FIBERS_DECL
int read(int fd, void * buf, unsigned nbytes, uint64_t offset = 0);

BOOST_FIBERS_DECL
int write(int fd, void const * buf, unsigned nbytes, uint64_t offset = 0);

BOOST_FIBERS_DECL
int pread(int fd, void * buf, unsigned nbytes, uint64_t offset);

BOOST_FIBERS_DECL
int pwrite(int fd, void const * buf, unsigned nbytes, uint64_t offset);

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif

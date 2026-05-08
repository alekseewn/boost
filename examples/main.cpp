#include <iostream>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/operations.hpp>
#include <boost/fiber/algo/work_stealing.hpp>
#include <boost/fiber/io_uring.hpp>

void fiber_task() {
    int fd = open("/tmp/boost_fiber_test", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) { perror("open"); return; }

    const char * msg1 = "hello from pwrite!";
    int rc = boost::fibers::io_uring::pwrite(fd, msg1, std::strlen(msg1), 0);
    std::cerr << "pwrite returned " << rc << std::endl;
    close(fd);

    fd = open("/tmp/boost_fiber_test", O_RDONLY);
    if (fd < 0) { perror("open"); return; }

    char buf[64] = {};
    int n = boost::fibers::io_uring::pread(fd, buf, sizeof(buf), 0);
    std::cerr << "pread returned " << n << " bytes: " << buf << std::endl;
    close(fd);

    unlink("/tmp/boost_fiber_test");
}

int main() {
    boost::fibers::use_scheduling_algorithm<
        boost::fibers::algo::work_stealing>(1);

    boost::fibers::fiber f(fiber_task);
    f.join();
    return 0;
}

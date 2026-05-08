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
    int fd = open("/tmp/boost_fiber_test", O_RDONLY);
    if (fd < 0) { perror("open"); return; }

    char buf[256] = {};
    int n = boost::fibers::io_uring::read(fd, buf, sizeof(buf));
    std::cerr << "read returned " << n << " bytes: " << buf << std::endl;
    close(fd);
}

int main() {
    int fd = open("/tmp/boost_fiber_test", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    const char * msg = "Hello from io_uring in Boost.Fiber!";
    write(fd, msg, std::strlen(msg));
    close(fd);

    boost::fibers::use_scheduling_algorithm<
        boost::fibers::algo::work_stealing>(1);

    boost::fibers::fiber f(fiber_task);
    boost::fibers::fiber g([](){
        int fn = open("test_file", O_CREAT | O_WRONLY | O_TRUNC, 0644);
        boost::fibers::io_uring::write(fn, "hello!!!!", 9);
    });
    g.join();
    f.join();

    unlink("/tmp/boost_fiber_test");
    return 0;
}

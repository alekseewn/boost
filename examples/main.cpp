#include <iostream>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/operations.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/fiber/condition_variable.hpp>
#include <boost/fiber/algo/work_stealing.hpp>
#include <thread>

struct Env {
    Env() {
        worker = std::thread(
        [this]{
            boost::fibers::use_scheduling_algorithm<boost::fibers::algo::work_stealing>(2);
            std::cout << "WORKER" << std::endl;
            mtx.lock();
            cv.wait(mtx);
            mtx.unlock();
        });

    }

    ~Env() {
        std::cout << "~ENV" << std::endl;
        cv.notify_all();
        worker.join();
    }

    std::thread worker;
    boost::fibers::mutex mtx;
    boost::fibers::condition_variable_any cv;

};

void fiber_func() {
    std::cout << "Hello from fiber!" << std::endl;
}

static Env env;

int main() {
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::work_stealing>(2);
    boost::fibers::fiber f(fiber_func);
    std::cout << "Hello from main!" << std::endl;
    f.join();
    return 0;
}

cmake_minimum_required(VERSION 3.14)
project(boost-example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_ROOT "../../boost_1_91_0")
set(Boost_LIBRARY_DIR "../../boost_1_91_0/stage/lib")
set(BOOST_INCLUDEDIR "../../boost_1_91_0")

find_package(Boost REQUIRED COMPONENTS fiber context)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE
    Boost::fiber
    Boost::context
)

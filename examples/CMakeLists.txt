cmake_minimum_required(VERSION 3.14)
project(boost-example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_ROOT "../../boost")
set(Boost_LIBRARY_DIR "../../boost/stage/lib")
set(BOOST_INCLUDEDIR "../../boost")

find_package(Boost REQUIRED COMPONENTS fiber context)

find_library(LIBURING_LIB uring)
if(NOT LIBURING_LIB)
    message(FATAL_ERROR "liburing not found")
endif()

add_executable(main main.cpp)

target_link_libraries(main PRIVATE
    Boost::fiber
    Boost::context
    ${LIBURING_LIB}
)

target_compile_definitions(main PRIVATE BOOST_FIBERS_USE_LIBURING)

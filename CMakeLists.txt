cmake_minimum_required(VERSION 3.6)
project(xnet)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES EventLoop.cpp EventLoop.h)
add_executable(test1 ${SOURCE_FILES} test/test1.cpp)
add_executable(test2 ${SOURCE_FILES} test/test2.cpp)
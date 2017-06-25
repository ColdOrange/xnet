cmake_minimum_required(VERSION 3.6)
project(xnet)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES EventLoop.cpp EventLoop.h Channel.cpp Channel.h Poller.cpp Poller.h)
add_executable(test3 ${SOURCE_FILES} test/test3.cpp)
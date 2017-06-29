cmake_minimum_required(VERSION 3.5)
project(xnet)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter -O2")

set(SOURCE_FILES
    EventLoop.cpp EventLoop.h
    Channel.cpp Channel.h
    Poller.cpp Poller.h
    Callbacks.h
    TimePoint.cpp TimePoint.h
    Timer.cpp Timer.h
    TimerId.h
    TimerQueue.cpp TimerQueue.h
    EventLoopThread.cpp EventLoopThread.h)

#add_executable(test3 ${SOURCE_FILES} test/test3.cpp)
#add_executable(test4 ${SOURCE_FILES} test/test4.cpp)
#add_executable(test5 ${SOURCE_FILES} test/test5.cpp)
add_executable(test6 ${SOURCE_FILES} test/test6.cpp)

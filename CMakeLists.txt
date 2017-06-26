cmake_minimum_required(VERSION 3.5)
project(xnet)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS
        "-std=c++11 -Wall -Wextra -Wno-unused-parameter -O2"
        )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS
        "-std=c++11 -Wall -Wextra -Wno-unused-parameter -O2 -pthread"
        )
endif()

set(SOURCE_FILES EventLoop.cpp EventLoop.h Channel.cpp Channel.h Poller.cpp Poller.h Callbacks.h TimePoint.cpp TimePoint.h Timer.cpp Timer.h TimerId.h TimerQueue.cpp TimerQueue.h)
add_executable(test4 ${SOURCE_FILES} test/test4.cpp)
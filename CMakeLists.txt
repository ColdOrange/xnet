cmake_minimum_required(VERSION 3.5)
project(xnet)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    set(CMAKE_CXX_FLAGS
        "-std=c++11 -Wall -Wextra -Wno-unused-parameter -O2")

    set(TIMER_QUEUE
        TimerQueue.cpp TimerQueue.h)

    set(EVENT_LOOP
        EventLoop.cpp EventLoop.h)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    set(CMAKE_CXX_FLAGS
        "-std=c++11 -Wall -Wextra -Wno-unused-parameter -O2 -pthread")

    set(TIMER_QUEUE
        linux/TimerQueue.cpp linux/TimerQueue.h)

    set(EVENT_LOOP
        linux/EventLoop.cpp linux/EventLoop.h)

endif()

set(SOURCE_FILES
    ${EVENT_LOOP}
    Channel.cpp Channel.h
    Poller.cpp Poller.h
    Callbacks.h
    TimePoint.cpp TimePoint.h
    Timer.cpp Timer.h
    TimerId.h
    ${TIMER_QUEUE})

add_executable(test4 ${SOURCE_FILES} test/test4.cpp)
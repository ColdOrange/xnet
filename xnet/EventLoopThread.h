//
// Created by Orange on 6/29/17.
//

#ifndef XNET_EVENTLOOPTHREAD_H
#define XNET_EVENTLOOPTHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>

namespace xnet {

class EventLoop;

class EventLoopThread
{
public:
    EventLoopThread();

    EventLoopThread(const EventLoopThread&) = delete;
    EventLoopThread& operator=(const EventLoopThread&) = delete;

    ~EventLoopThread();

    EventLoop* startLoop();

private:
    EventLoop* eventLoop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;

    void threadFunc();
};

} // namespace xnet

#endif // XNET_EVENTLOOPTHREAD_H

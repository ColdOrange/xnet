//
// Created by Orange on 6/29/17.
//

#include "EventLoopThread.h"

using namespace xnet;

EventLoopThread::EventLoopThread()
    : eventLoop_(nullptr),
      exiting_(false),
      thread_([this] { threadFunc(); })
{ }

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    eventLoop_->quit();
    thread_.join();
}

EventLoop* EventLoopThread::startLoop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (eventLoop_ == nullptr) {
            cond_.wait(lock);
        }
    }
    return eventLoop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop eventLoop;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        eventLoop_ = &eventLoop;
    }
    cond_.notify_one();
    eventLoop.loop();
}

//
// Created by Orange on 6/23/17.
//

#include <poll.h>
#include <assert.h>
#include <iostream>

#include "EventLoop.h"

namespace xnet {

__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop()
    : looping_(false), threadId_(std::this_thread::get_id())
{
    //LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    std::cout << "EventLoop created " << this << " in thread " << threadId_ << "\n";
    if (t_loopInThisThread) {
        //LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
        std::cout << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_ << "\n";
        exit(1);
    }
    else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    poll(nullptr, 0, 5000);

    //LOG_TRACE << "EventLoop " << this << " stop looping";
    std::cout << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

void EventLoop::assertInLoopThread()
{
    if (!isInLoopThread()) {
        abortNotInLoopThread();
    }
}

bool EventLoop::isInLoopThread() const
{
    return threadId_ == std::this_thread::get_id();
}

void EventLoop::abortNotInLoopThread()
{
    //LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
    //          << " was created in threadId_ = " << threadId_
    //          << ", current thread id = " <<  CurrentThread::tid();
    std::cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " <<  std::this_thread::get_id() << "\n";
    exit(1);
}

} // namespace xnet

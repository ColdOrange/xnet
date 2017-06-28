//
// Created by Orange on 6/23/17.
//

#include <assert.h>
#include <iostream>

#include "EventLoop.h"
#include "../Channel.h"

using namespace xnet;

__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeoutMs = 10000;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(std::this_thread::get_id()),
      poller_(new Poller(this)),
      pollReturnTime_(TimePoint::invalid()),
      timerQueue_(new TimerQueue(this))
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

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeoutMs, &activeChannels_);
        for (const auto& channel : activeChannels_) {
            channel->handleEvent();
        }
    }

    //LOG_TRACE << "EventLoop " << this << " stop looping";
    std::cout << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
}

TimerId EventLoop::runAt(const TimePoint& timePoint, const TimerCallback& cb)
{
  return timerQueue_->addTimer(cb, timePoint, 0.0);
}

TimerId EventLoop::runAfter(double delaySeconds, const TimerCallback& cb)
{
  TimePoint timePoint(addTime(TimePoint::now(), delaySeconds));
  return timerQueue_->addTimer(cb, timePoint, 0.0);
}

TimerId EventLoop::runEvery(double intervalSeconds, const TimerCallback& cb)
{
  TimePoint timePoint(addTime(TimePoint::now(), intervalSeconds));
  return timerQueue_->addTimer(cb, timePoint, intervalSeconds);
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
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

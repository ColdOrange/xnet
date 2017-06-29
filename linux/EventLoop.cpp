//
// Created by Orange on 6/23/17.
//

#include <assert.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <iostream>

#include "EventLoop.h"

namespace xnet {

namespace detail {

int createEventFd()
{
    int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventFd < 0) {
        //LOG_SYSERR << "Failed in eventfd";
        std::cout << "Failed in eventfd\n";
        exit(1);
    }
    return eventFd;
}

} // namespace detail

} // namespace xnet

using namespace xnet;
using namespace xnet::detail;

__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeoutMs = 10000;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(std::this_thread::get_id()),
      poller_(new Poller(this)),
      pollReturnTime_(TimePoint::invalid()),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventFd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
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

    wakeupChannel_->setReadCallback([this] { handleRead(); });
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    ::close(wakeupFd_);
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
        doPendingFunctors();
    }

    //LOG_TRACE << "EventLoop " << this << " stop looping";
    std::cout << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor& cb)
{
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        std::lock_guard<std::mutex> guard(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
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

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        //LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
        std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
        exit(1);
    }
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

void EventLoop::handleRead()
{
    uint64_t one;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        //LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
        std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
        exit(1);
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> guard(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const auto& cb : functors) {
        cb();
    }
    callingPendingFunctors_ = false;
}

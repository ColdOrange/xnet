//
// Created by Orange on 6/23/17.
//

#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>

#include "EventLoop.h"

using namespace xnet;

__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
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

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, wakeupFd_) < 0) {
        //LOG_SYSFATAL << "Failed in socketpair";
        std::cout << "Failed in socketpair\n";
        exit(1);
    }
    wakeupChannel_.reset(new Channel(this, wakeupFd_[0]));
    wakeupChannel_->setReadCallback([this] { handleRead(); });
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    ::close(wakeupFd_[0]);
    ::close(wakeupFd_[1]);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(timerQueue_->getPollTimeoutMs(), &activeChannels_);
        for (const auto& channel : activeChannels_) {
            channel->handleEvent();
        }
        timerQueue_->handleTimers();
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
    ssize_t n = ::write(wakeupFd_[1], &one, sizeof(one));
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

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
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
    ssize_t n = ::read(wakeupFd_[0], &one, sizeof(one));
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

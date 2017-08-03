//
// Created by Orange on 6/25/17.
//

#include <assert.h>
#include <iostream>

#include "TimerQueue.h"
#include "EventLoop.h"
#include "TimerId.h"

namespace xnet {

namespace detail {

int howMuchTimeFromNow(const TimePoint& timePoint)
{
    int64_t microSeconds = timePoint.microSecondsSinceEpoch() - TimePoint::now().microSecondsSinceEpoch();
    if (microSeconds < 1000) {
        microSeconds = 1000;
    }
    return static_cast<int>(microSeconds / 1000);
}

} // namespace detail

} // namespace xnet

using namespace xnet;
using namespace xnet::detail;

TimerQueue::TimerQueue(EventLoop* loop)
    : ownerLoop_(loop),
      callingExpiredTimers_(false)
{ }

TimerQueue::~TimerQueue()
{
    for (auto& entry : timers_) {
        delete entry.second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb, const TimePoint& timePoint, double intervalSeconds)
{
    Timer* timer = new Timer(cb, timePoint, intervalSeconds);
    ownerLoop_->runInLoop([this, timer] {
        addTimerInLoop(timer);
    });
    return TimerId(timer, timer->sequence());;
}

void TimerQueue::cancel(TimerId timerId)
{
    ownerLoop_->runInLoop([this, timerId] {
        cancelInLoop(timerId);
    });
}

int TimerQueue::getPollTimeoutMs() const
{
    ownerLoop_->assertInLoopThread();
    if (timers_.empty()) {
        return kMaxPollTimeoutMs;
    }
    else {
        return howMuchTimeFromNow(timers_.begin()->second->expiration());
    }
};

void TimerQueue::handleTimers()
{
    ownerLoop_->assertInLoopThread();
    TimePoint now(TimePoint::now());
    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancellingTimers_.clear();
    // Safe to callback outside critical section
    for (const auto& entry : expired) {
        entry.second->run();
    }
    callingExpiredTimers_ = false;

    reset(expired, now);
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    ownerLoop_->assertInLoopThread();
    timers_.insert(std::make_pair(timer->expiration(), timer));
    activeTimers_.insert(std::make_pair(timer, timer->sequence()));
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    ownerLoop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());

    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    auto it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        delete it->first;
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_) { // this timer is being expired (now in expired vector)
        cancellingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(const TimePoint& now)
{
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;

    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    for (auto& entry : expired) {
        ActiveTimer timer(entry.second, entry.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1);
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, const TimePoint& now)
{
    for (auto& entry : expired) {
        ActiveTimer timer(entry.second, entry.second->sequence());
        if (entry.second->repeat() && cancellingTimers_.find(timer) == cancellingTimers_.end()) {
            entry.second->restart(now);
            timers_.insert(std::make_pair(entry.second->expiration(), entry.second));
        }
        else {
            delete entry.second;
        }
    }
}

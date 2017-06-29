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
      timers_()
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
    return TimerId(timer);
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
    // Safe to callback outside critical section
    for (const auto& entry : expired) {
        entry.second->run();
    }
    reset(expired, now);
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    ownerLoop_->assertInLoopThread();
    timers_.insert(std::make_pair(timer->expiration(), timer));
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(const TimePoint& now)
{
    std::vector<Entry> expired;

    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, const TimePoint& now)
{
    for (auto& entry : expired) {
        if (entry.second->repeat()) {
            entry.second->restart(now);
            timers_.insert(std::make_pair(entry.second->expiration(), entry.second));
        }
        else {
            delete entry.second;
        }
    }
}

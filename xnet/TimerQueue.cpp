//
// Created by Orange on 6/25/17.
//

#include <assert.h>
#include <unistd.h>
#include <strings.h>
#include <sys/timerfd.h>
#include <iostream>

#include "EventLoop.h"
#include "TimerQueue.h"
#include "TimerId.h"

namespace xnet {

namespace detail {

int createTimerFd()
{
    int timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerFd < 0) {
        //LOG_SYSFATAL << "Failed in timerfd_create";
        std::cout << "Failed in timerfd_create\n";
    }
    return timerFd;
}

struct timespec howMuchTimeFromNow(const TimePoint& timePoint)
{
    int64_t microSeconds = timePoint.microSecondsSinceEpoch() - TimePoint::now().microSecondsSinceEpoch();
    if (microSeconds < 100) {
        microSeconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microSeconds / TimePoint::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microSeconds % TimePoint::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void readTimerFd(int timerFd, const TimePoint& now)
{
    uint64_t expiredTimes;
    ssize_t n = ::read(timerFd, &expiredTimes, sizeof(expiredTimes));
    //LOG_TRACE << "TimerQueue::handleRead() " << expiredTimes << " times at " << now.toString();
    std::cout << "TimerQueue::handleRead() " << expiredTimes << " times at " << now.toFormattedString() << "\n";
    if (n != sizeof(expiredTimes)) {
        //LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
        std::cout << "TimerQueue::handleRead() reads " << n << " bytes instead of 8\n";
    }
}

void resetTimerFd(int timerFd, const TimePoint& expiration)
{
    // Wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerFd, 0, &newValue, &oldValue);
    if (ret) {
        //LOG_SYSERR << "timerfd_settime()";
        std::cout << "timerfd_settime()\n";
    }
}

} // namespace detail

} // namespace xnet

using namespace xnet;
using namespace xnet::detail;

TimerQueue::TimerQueue(EventLoop* loop)
    : ownerLoop_(loop),
      timerFd_(createTimerFd()),
      timerFdChannel_(loop, timerFd_),
      callingExpiredTimers_(false)
{
    timerFdChannel_.setReadCallback([this](const TimePoint&) { handleRead(); });
    timerFdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timerFd_);
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
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    ownerLoop_->runInLoop([this, timerId] {
        cancelInLoop(timerId);
    });
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    ownerLoop_->assertInLoopThread();
    bool firstToExpire = insert(timer);
    if (firstToExpire) {
        resetTimerFd(timerFd_, timer->expiration());
    }
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
    else if (callingExpiredTimers_) { // this timer is being expired (e.g. in expired vector)
        cancellingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead()
{
    ownerLoop_->assertInLoopThread();

    TimePoint now(TimePoint::now());
    readTimerFd(timerFd_, now);

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

std::vector<TimerQueue::Entry> TimerQueue::getExpired(const TimePoint& now)
{
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
            insert(entry.second);
        }
        else {
            delete entry.second;
        }
    }

    if (!timers_.empty()) {
        const TimePoint& nextToExpire = timers_.begin()->second->expiration();
        resetTimerFd(timerFd_, nextToExpire);
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool firstToExpire = false;
    auto oldFirst = timers_.begin();
    if (oldFirst == timers_.end() || timer->expiration() < oldFirst->first) {
        firstToExpire = true;
    }

    {
        auto ret = timers_.insert(std::make_pair(timer->expiration(), timer));
        assert(ret.second);
    }
    {
        auto ret = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(ret.second);
    }
    assert(timers_.size() == activeTimers_.size());
    return firstToExpire;
}

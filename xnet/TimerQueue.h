//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMERQUEUE_H
#define XNET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "TimePoint.h"
#include "Callbacks.h"
#include "Channel.h"
#include "Timer.h"
#include "TimerId.h"

namespace xnet {

class EventLoop;

//
// A best efforts timer queue.
// No guarantee that the callback will be on time.
//
class TimerQueue
{
public:
    TimerQueue(EventLoop* loop);

    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    ~TimerQueue();

    TimerId addTimer(const TimerCallback& cb, const TimePoint& timePoint, double intervalSeconds);
    void cancel(TimerId timerId);

    int getPollTimeoutMs() const;
    void handleTimers();

private:
    // Use TimePoint and ptr of Timer as Key,
    // in case there are Timers which have the same expiration TimePoint
    typedef std::pair<TimePoint, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    // Same as TimerList, for cancelling
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    const int kMaxPollTimeoutMs = 10000;

    EventLoop* ownerLoop_;
    TimerList  timers_;
    ActiveTimerSet activeTimers_;
    bool callingExpiredTimers_; // this timer is being expired
    ActiveTimerSet cancellingTimers_;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    std::vector<Entry> getExpired(const TimePoint& now);
    void reset(const std::vector<Entry>& expired, const TimePoint& now);
};

}

#endif // XNET_TIMERQUEUE_H

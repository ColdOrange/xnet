//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMERQUEUE_H
#define XNET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "Callbacks.h"
#include "Channel.h"
#include "TimePoint.h"
#include "Timer.h"

namespace xnet {

class EventLoop;
class TimerId;

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

    //
    // Schedules the callback to be run at given time,
    // repeats if interval > 0.0.
    //
    // Must be thread safe. Usually be called from other threads.
    TimerId addTimer(const TimerCallback& cb, const TimePoint& timePoint, double intervalSeconds);

    void cancel(TimerId timerId);

private:
    // Use TimePoint and ptr of Timer as Key,
    // in case there are Timers which have the same expiration TimePoint
    typedef std::pair<TimePoint, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    // Same as TimerList, for cancelling
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    EventLoop* ownerLoop_;
    const int  timerFd_;
    Channel    timerFdChannel_;
    TimerList  timers_;
    // For cancelling
    ActiveTimerSet activeTimers_;
    bool callingExpiredTimers_; // this timer is being expired
    ActiveTimerSet cancellingTimers_;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    // Called when timerFd_ alarms
    void handleRead();
    // Move out all expired timers
    std::vector<Entry> getExpired(const TimePoint& now);

    void reset(const std::vector<Entry>& expired, const TimePoint& now);

    bool insert(Timer* timer);
};

}

#endif // XNET_TIMERQUEUE_H

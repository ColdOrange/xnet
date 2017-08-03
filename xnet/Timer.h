//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMER_H
#define XNET_TIMER_H

#include <stdint.h>
#include <atomic>

#include "Callbacks.h"
#include "TimePoint.h"

namespace xnet {

class Timer
{
public:
    Timer(const TimerCallback& cb, const TimePoint& timePoint, double intervalSeconds)
    : callback_(cb),
      expiration_(timePoint),
      interval_(intervalSeconds),
      repeat_(intervalSeconds > 0.0),
      sequence_(numCreated_.fetch_add(1))
    { }

    void run() const { callback_(); }

    TimePoint expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(const TimePoint& now);

    static int64_t numCreated() { return numCreated_.load(); }

private:
    TimerCallback callback_;
    TimePoint     expiration_;
    double        interval_;
    bool          repeat_;
    int64_t       sequence_;

    static std::atomic<int64_t> numCreated_;
};

}

#endif // XNET_TIMER_H

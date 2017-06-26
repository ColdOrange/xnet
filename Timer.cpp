//
// Created by Orange on 6/25/17.
//

#include "Timer.h"

using namespace xnet;

Timer::Timer(const TimerCallback& cb, const TimePoint& timePoint, double intervalSeconds)
    : callback_(cb),
      expiration_(timePoint),
      interval_(intervalSeconds),
      repeat_(intervalSeconds > 0.0)
{ }

void Timer::run() const
{
    callback_();
}

void Timer::restart(const TimePoint& now)
{
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    }
    else {
        expiration_ = TimePoint::invalid();
    }
}

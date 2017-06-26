//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMER_H
#define XNET_TIMER_H

#include "Callbacks.h"
#include "TimePoint.h"

namespace xnet {

class Timer
{
public:
    Timer(const TimerCallback& cb, const TimePoint& timePoint, double intervalSeconds);

    void run() const;

    TimePoint expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }

    void restart(const TimePoint& now);

private:
    TimerCallback callback_;
    TimePoint     expiration_;
    double        interval_;
    bool          repeat_;
};

}

#endif // XNET_TIMER_H

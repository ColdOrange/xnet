//
// Created by Orange on 6/25/17.
//

#include "Timer.h"

using namespace xnet;

std::atomic<int64_t> Timer::numCreated_;

void Timer::restart(TimePoint now)
{
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    }
    else {
        expiration_ = TimePoint::invalid();
    }
}

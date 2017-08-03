//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMERID_H
#define XNET_TIMERID_H

#include <stdint.h>

namespace xnet {

class Timer;

//
// An opaque identifier, for canceling Timer.
//
class TimerId
{
    friend class TimerQueue;

public:
    TimerId()
        : timer_(nullptr),
          sequence_(0)
    { }

    TimerId(Timer* timer, int64_t seq)
        : timer_(timer),
          sequence_(seq)
    { }

    // default copy-ctor, dtor and assignment are okay

private:
    Timer* timer_;
    int64_t sequence_;
};

}

#endif // XNET_TIMERID_H

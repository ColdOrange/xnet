//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMERID_H
#define XNET_TIMERID_H

#include <stdint.h>

#include "Copyable.h"
#include "Timer.h"

namespace xnet {

//
// An opaque identifier, for canceling Timer.
//
class TimerId : public Copyable
{
public:
    TimerId()
        : timer_(nullptr),
          sequence_(0)
    { }

    TimerId(Timer* timer, int64_t seq)
        : timer_(timer),
          sequence_(seq)
    { }

    // Implicit copy-ctor, move-ctor, dtor and assignment are fine.

    friend class TimerQueue;

private:
    Timer* timer_;
    int64_t sequence_;
};

}

#endif // XNET_TIMERID_H

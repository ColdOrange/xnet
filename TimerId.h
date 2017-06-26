//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMERID_H
#define XNET_TIMERID_H

namespace xnet {

class Timer;

//
// An opaque identifier, for canceling Timer.
//
class TimerId
{
public:
    explicit TimerId(Timer* timer)
        : timer_(timer)
    { }

    TimerId(const TimerId&) = default;
    TimerId& operator=(const TimerId&) = default;

private:
    Timer* timer_;
};

}

#endif // XNET_TIMERID_H

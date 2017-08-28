//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMEPOINT_H
#define XNET_TIMEPOINT_H

#include <time.h>
#include <stdint.h>
#include <chrono>
#include <string>

#include "Copyable.h"

namespace xnet {

class TimePoint : public Copyable
{
public:
    static const int kMilliSecondsPerSecond = 1000;
    static const int kMicroSecondsPerSecond = 1000 * 1000;

    explicit TimePoint(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch)
    { }

    // Implicit copy-ctor, move-ctor, dtor and assignment are fine.

    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const
    {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    bool valid() { return microSecondsSinceEpoch_ > 0; }

    std::string toString() const;
    std::string toFormattedString(bool showMicroSeconds = true) const;

    static TimePoint now();
    static TimePoint invalid();

private:
    int64_t microSecondsSinceEpoch_;
};

bool operator<(TimePoint lhs, TimePoint rhs);
bool operator==(TimePoint lhs, TimePoint rhs);

// Gets time difference of two TimePoints, result in seconds.
double timeDifference(TimePoint high, TimePoint low);
TimePoint addTime(TimePoint timePoint, double seconds);

} // namespace xnet

#endif // XNET_TIMEPOINT_H

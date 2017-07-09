//
// Created by Orange on 6/25/17.
//

#ifndef XNET_TIMEPOINT_H
#define XNET_TIMEPOINT_H

#include <time.h>
#include <stdint.h>
#include <chrono>
#include <string>

namespace xnet {

class TimePoint
{
public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

    explicit TimePoint(int64_t microSecondsSinceEpoch);

    TimePoint(const TimePoint&) = default;
    TimePoint& operator=(const TimePoint&) = default;

    int64_t microSecondsSinceEpoch() const;
    time_t secondsSinceEpoch() const;

    bool valid();

    std::string toString() const;
    std::string toFormattedString(bool showMicroSeconds = true) const;

    static TimePoint now();
    static TimePoint invalid();

private:
    using Clock = std::chrono::system_clock;
    Clock::time_point timePoint_;

    TimePoint(Clock::time_point timePoint);
};

bool operator<(const TimePoint& lhs, const TimePoint& rhs);
bool operator==(const TimePoint& lhs, const TimePoint& rhs);

// Gets time difference of two TimePoints, result in seconds.
double timeDifference(const TimePoint& high, const TimePoint& low);
TimePoint addTime(const TimePoint& timePoint, double seconds);

} // namespace xnet

#endif // XNET_TIMEPOINT_H

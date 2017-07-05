//
// Created by Orange on 6/25/17.
//

#include <inttypes.h>

#include "TimePoint.h"

using namespace xnet;

TimePoint::TimePoint(int64_t microSecondsSinceEpoch)
    : timePoint_(std::chrono::microseconds(microSecondsSinceEpoch))
{ }

TimePoint::TimePoint(Clock::time_point timePoint)
    : timePoint_(timePoint)
{ }

int64_t TimePoint::microSecondsSinceEpoch() const
{
    return std::chrono::duration_cast<std::chrono::microseconds>(timePoint_.time_since_epoch()).count();
}

time_t TimePoint::secondsSinceEpoch() const
{
    return static_cast<time_t>(microSecondsSinceEpoch() / kMicroSecondsPerSecond);
}

bool TimePoint::valid()
{
    return microSecondsSinceEpoch() > 0;
}

std::string TimePoint::toString() const
{
    char buf[32] = { 0 };
    int64_t seconds = microSecondsSinceEpoch() / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch() % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string TimePoint::toFormattedString(bool showMicroSeconds) const
{
    char buf[32] = { 0 };
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch() / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroSeconds) {
        int microseconds = static_cast<int>(microSecondsSinceEpoch() % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                 microseconds);
    }
    else {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

TimePoint TimePoint::now()
{
    return TimePoint(Clock::now());
}

TimePoint TimePoint::invalid()
{
    return TimePoint(0);
}

bool xnet::operator<(const TimePoint& lhs, const TimePoint& rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

bool xnet::operator==(const TimePoint& lhs, const TimePoint& rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

double xnet::timeDifference(const TimePoint& high, const TimePoint& low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff / TimePoint::kMicroSecondsPerSecond);
}

TimePoint xnet::addTime(const TimePoint& timePoint, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * TimePoint::kMicroSecondsPerSecond);
    return TimePoint(timePoint.microSecondsSinceEpoch() + delta);
}

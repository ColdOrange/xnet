//
// Created by Orange on 6/25/17.
//

#include <inttypes.h>
#include <sys/time.h>

#include "TimePoint.h"

using namespace xnet;

std::string TimePoint::toString() const
{
    char buf[32];
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string TimePoint::toFormattedString(bool showMicroSeconds) const
{
    char buf[32];
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch() / kMicroSecondsPerSecond);
    struct tm tm_time;
    ::gmtime_r(&seconds, &tm_time);

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
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return TimePoint(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

TimePoint TimePoint::invalid()
{
    return TimePoint(0);
}

bool xnet::operator<(TimePoint lhs, TimePoint rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

bool xnet::operator==(TimePoint lhs, TimePoint rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

double xnet::timeDifference(TimePoint high, TimePoint low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff / TimePoint::kMicroSecondsPerSecond);
}

TimePoint xnet::addTime(TimePoint timePoint, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * TimePoint::kMicroSecondsPerSecond);
    return TimePoint(timePoint.microSecondsSinceEpoch() + delta);
}

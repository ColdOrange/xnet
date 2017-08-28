//
// Created by Orange on 8/27/17.
//

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sstream>
#include <thread>

#include "Logging.h"

namespace xnet {

__thread char t_errnobuf[512];
__thread char t_time[32];
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::LogLevel initLogLevel()
{
    if (::getenv("XNET_LOG_TRACE")) {
        return Logger::TRACE;
    }
    else {
        return Logger::DEBUG;
    }
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL "
};

void defaultOutput(const char* msg, size_t len)
{
    size_t n = ::fwrite(msg, 1, len, stdout);
    assert(n == len);
}

void defaultFlush()
{
    fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

} // namespace xnet

using namespace xnet;

Logger::Impl::Impl(LogLevel level, int savedErrno, const char* file, int line)
    : time_(TimePoint::now()),
      level_(level),
      line_(line),
      fullname_(file),
      basename_(nullptr)
{
    const char* separator = strrchr(fullname_, '/');
    basename_ = (separator != nullptr) ? separator + 1 : fullname_;

    formatTime();
    std::ostringstream tid;
    tid << std::this_thread::get_id();
    stream_ << tid.str() << " " << T(LogLevelName[level], 6);
    if (savedErrno != 0) {
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

void Logger::Impl::formatTime()
{
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / TimePoint::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % TimePoint::kMicroSecondsPerSecond);

    if (seconds != t_lastSecond) {
        t_lastSecond = seconds;
        struct tm tm_time;
        ::gmtime_r(&seconds, &tm_time);

        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17);
    }
    Fmt us(".%06d ", microseconds);
    assert(us.length() == 8);
    stream_ << T(t_time, 17) << T(us.data(), 8);
}

void Logger::Impl::finish()
{
    stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(const char* file, int line)
    : impl_(INFO, 0, file, line)
{ }

Logger::Logger(const char* file, int line, LogLevel level)
    : impl_(level, 0, file, line)
{ }

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}

Logger::Logger(const char* file, int line, bool toAbort)
    : impl_(toAbort ? FATAL : ERROR, errno, file, line)
{ }

Logger::~Logger()
{
    impl_.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush;
}

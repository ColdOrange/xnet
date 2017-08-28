//
// Created by Orange on 8/27/17.
//

#ifndef XNET_LOGGING_H
#define XNET_LOGGING_H

#include <memory>
#include <functional>

#include "Noncopyable.h"
#include "LogStream.h"
#include "TimePoint.h"

namespace xnet {

class Logger : Noncopyable
{
public:
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };

    Logger(const char* file, int line);
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    Logger(const char* file, int line, bool toAbort);

    ~Logger();

    LogStream& stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef std::function<void(const char* msg, size_t len)> OutputFunc;
    typedef std::function<void()> FlushFunc;

    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);

private:
    class Impl : Noncopyable
    {
    public:
        Impl(LogLevel level, int savedErrno, const char* file, int line);

        void formatTime();

        void finish();

        TimePoint time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        const char* fullname_;
        const char* basename_;
    };

    Impl impl_;
};

#define LOG_TRACE \
    if (Logger::logLevel() <= Logger::TRACE) \
        Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()

#define LOG_DEBUG \
    if (Logger::logLevel() <= Logger::DEBUG) \
        Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()

#define LOG_INFO \
    if (Logger::logLevel() <= Logger::INFO) \
        Logger(__FILE__, __LINE__).stream()

#define LOG_WARN     Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR    Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL    Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR   Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This is very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template<typename T>
T* CheckNotNull(const char* file, int line, const char* names, T* ptr)
{
    if (ptr == NULL) {
        Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}

} // namespace xnet

#endif // XNET_LOGGING_H

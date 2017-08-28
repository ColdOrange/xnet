//
// Created by Orange on 8/27/17.
//

#ifndef XNET_LOGFILE_H
#define XNET_LOGFILE_H

#include <string>
#include <memory>
#include <mutex>

#include "Noncopyable.h"
#include "File.h"

namespace xnet {

class LogFile : Noncopyable
{
public:
    LogFile(const std::string& basename,
            size_t rollSize,
            bool threadSafe = false,
            int flushInterval = 3);

    void append(const char* logLine, size_t len);

    void flush();

private:
    static const int kCheckTimeRoll_ = 1024;
    static const int kRollPerSeconds_ = 60 * 60 * 24;

    const std::string basename_;
    const size_t rollSize_;
    const int flushInterval_;
    std::unique_ptr<AppendableFile> file_;
    std::unique_ptr<std::mutex> mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    int count_;

    static std::string getLogFileName(const std::string& basename, time_t* now);

    void append_unlocked(const char* logLine, size_t len);

    void rollFile();
};

} // namespace xnet

#endif // XNET_LOGFILE_H

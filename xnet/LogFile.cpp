//
// Created by Orange on 8/27/17.
//

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "LogFile.h"

using namespace xnet;

LogFile::LogFile(const std::string& basename,
                 size_t rollSize,
                 bool threadSafe,
                 int flushInterval)
    : basename_(basename),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      mutex_(threadSafe ? new std::mutex() : nullptr),
      startOfPeriod_(0),
      lastRoll_(0),
      lastFlush_(0),
      count_(0)
{
    assert(basename.find('/') == std::string::npos);
    rollFile();
}

void LogFile::append(const char* logLine, size_t len)
{
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        append_unlocked(logLine, len);
    }
    else {
        append_unlocked(logLine, len);
    }
}

void LogFile::flush()
{
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        file_->flush();
    }
    else {
        file_->flush();
    }
}

void LogFile::append_unlocked(const char* logLine, size_t len)
{
    file_->append(logLine, len);

    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    }
    else {
        if (count_ > kCheckTimeRoll_) {
            count_ = 0;
            time_t now = ::time(nullptr);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod_ != startOfPeriod_) {
                rollFile();
            }
            else if (now - lastFlush_ > flushInterval_) {
                lastFlush_ = now;
                file_->flush();
            }
        }
        else {
            ++count_;
        }
    }
}

void LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendableFile(filename));
    }
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
    std::string filename;
    filename.reserve(basename.size() + 32);
    filename = basename;

    char timebuf[32];
    char pidbuf[32];
    struct tm tm;
    *now = time(nullptr);
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;
    snprintf(pidbuf, sizeof(pidbuf), ".%d", ::getpid());
    filename += pidbuf;
    filename += ".log";

    return filename;
}

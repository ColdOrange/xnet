//
// Created by Orange on 8/27/17.
//

#ifndef XNET_ASYNCLOGGING_H
#define XNET_ASYNCLOGGING_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "Noncopyable.h"
#include "LogStream.h"
#include "CountDownLatch.h"

namespace xnet {

class AsyncLogging : Noncopyable
{
public:
    AsyncLogging(const std::string& basename, size_t rollSize, int flushInterval = 3);

    ~AsyncLogging();

    void append(const char* logLine, size_t len);

private:
    typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferVector;

    const std::string basename_;
    const size_t rollSize_;
    const int flushInterval_;
    std::atomic<bool> running_;
    std::thread thread_;
    CountDownLatch latch_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;

    void start();
    void stop();
    void threadFunc();
};

} // namespace xnet

#endif // XNET_ASYNCLOGGING_H

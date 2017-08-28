//
// Created by Orange on 8/27/17.
//

#include <stdio.h>
#include <assert.h>
#include <chrono>
#include <type_traits>

#include "AsyncLogging.h"
#include "LogFile.h"
#include "TimePoint.h"

using namespace xnet;

AsyncLogging::AsyncLogging(const std::string& basename, size_t rollSize, int flushInterval)
    : basename_(basename),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      running_(false),
      thread_(),
      latch_(1),
      mutex_(),
      cond_(),
      currentBuffer_(new Buffer()),
      nextBuffer_(new Buffer()),
      buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
    start();
}

AsyncLogging::~AsyncLogging()
{
    if (running_) {
        stop();
    }
}

void AsyncLogging::append(const char* logLine, size_t len)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (currentBuffer_->available() > len) {
        currentBuffer_->append(logLine, len);
    }
    else {
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else {
            currentBuffer_.reset(new Buffer());
        }
        currentBuffer_->append(logLine, len);
        cond_.notify_one();
    }
}

void AsyncLogging::start()
{
    running_ = true;
    thread_ = std::thread([this] { threadFunc(); });
    latch_.wait(); // make sure that stop() will not execute before threadFunc()
}

void AsyncLogging::stop()
{
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

void AsyncLogging::threadFunc()
{
    assert(running_);
    latch_.countDown();

    LogFile output(basename_, rollSize_);
    BufferPtr newBuffer1(new Buffer());
    BufferPtr newBuffer2(new Buffer());
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    while (running_) {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (buffers_.empty()) {
                cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_) {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25) {
            char buf[256];
            snprintf(buf, sizeof(buf), "Dropped log messages at %s, %zd larger buffers\n",
                     TimePoint::now().toFormattedString().c_str(), buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, strlen(buf));
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        for (size_t i = 0; i < buffersToWrite.size(); ++i) {
            output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }

        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }

        if (!newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        if (!newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}

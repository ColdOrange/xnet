//
// Created by Orange on 8/27/17.
//

#ifndef XNET_COUNTDOWNLATCH_H
#define XNET_COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>

#include "Noncopyable.h"

namespace xnet {

class CountDownLatch : Noncopyable
{
public:
    explicit CountDownLatch(int count)
        : count_(count)
    { }

    void wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (count_ > 0) {
            cond_.wait(lock);
        }
    }

    void countDown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        --count_;
        if (count_ == 0) {
            cond_.notify_all();
        }
    }

    int getCount() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    int count_;
};

} // namespace xnet

#endif // XNET_COUNTDOWNLATCH_H

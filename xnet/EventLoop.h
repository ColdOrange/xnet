//
// Created by Orange on 6/23/17.
//

#ifndef XNET_EVENTLOOP_H
#define XNET_EVENTLOOP_H

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>

#include "Channel.h"
#include "Poller.h"
#include "TimerId.h"
#include "TimerQueue.h"

namespace xnet {

class EventLoop
{
public:
    typedef std::function<void()> Functor;

    EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    virtual ~EventLoop();

    void loop();
    void quit();

    // Time when poll returns, usually means data arrival.
    TimePoint pollReturnTime() const { return pollReturnTime_; }

    // Runs callback immediately in the loop thread.
    // It wakes up the loop, and run the cb.
    // If in the same loop thread, cb is run within the function.
    // Safe to call from other threads.
    void runInLoop(const Functor& cb);

    // Queues callback in the loop thread.
    // Safe to call from other threads.
    void queueInLoop(const Functor& cb);

    // Timers
    // Safe to call from other threads.
    TimerId runAt(const TimePoint& timePoint, const TimerCallback& cb);
    TimerId runAfter(double delaySeconds, const TimerCallback& cb);
    TimerId runEvery(double intervalSeconds, const TimerCallback& cb);
    void cancel(TimerId timerId);

    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread();
    bool isInLoopThread() const;

private:
    typedef std::vector<Channel*> ChannelList;

    bool looping_; // atomic
    bool quit_; // atomic
    bool callingPendingFunctors_; // atomic
    std::thread::id threadId_;
    std::unique_ptr<Poller> poller_;
    TimePoint pollReturnTime_;
    std::unique_ptr<TimerQueue> timerQueue_;
    ChannelList activeChannels_;
    int wakeupFd_[2];
    std::unique_ptr<Channel> wakeupChannel_;
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_; // guarded by mutex_

    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();
};

} // namespace xnet

#endif // XNET_EVENTLOOP_H

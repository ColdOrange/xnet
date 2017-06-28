//
// Created by Orange on 6/23/17.
//

#ifndef XNET_EVENTLOOP_H
#define XNET_EVENTLOOP_H

#include <vector>
#include <memory>
#include <thread>

#include "TimerQueue.h"
#include "../Poller.h"
#include "../TimerId.h"

namespace xnet {

class Channel;

class EventLoop
{
public:
    EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    virtual ~EventLoop();

    void loop();
    void quit();

    // Time when poll returns, usually means data arrival.
    TimePoint pollReturnTime() const { return pollReturnTime_; }

    // Timers
    TimerId runAt(const TimePoint& timePoint, const TimerCallback& cb);
    TimerId runAfter(double delaySeconds, const TimerCallback& cb);
    TimerId runEvery(double intervalSeconds, const TimerCallback& cb);
    //void cancel(TimerId timerId);

    void updateChannel(Channel* channel);

    void assertInLoopThread();
    bool isInLoopThread() const;

private:
    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    bool quit_;
    std::thread::id threadId_;
    std::unique_ptr<Poller> poller_;
    TimePoint pollReturnTime_;
    std::unique_ptr<TimerQueue> timerQueue_;
    ChannelList activeChannels_;

    void abortNotInLoopThread();
};

} // namespace xnet

#endif // XNET_EVENTLOOP_H

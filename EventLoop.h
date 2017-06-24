//
// Created by Orange on 6/23/17.
//

#ifndef XNET_EVENTLOOP_H
#define XNET_EVENTLOOP_H

#include <vector>
#include <memory>
#include <thread>

#include "Poller.h"

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

    void updateChannel(Channel* channel);

    void assertInLoopThread();
    bool isInLoopThread() const;

private:
    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    bool quit_;
    std::thread::id threadId_;
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;

    void abortNotInLoopThread();
};

} // namespace xnet

#endif // XNET_EVENTLOOP_H

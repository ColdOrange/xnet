//
// Created by Orange on 6/24/17.
//

#ifndef XNET_POLLER_H
#define XNET_POLLER_H

#include <poll.h>
#include <map>
#include <vector>
#include <chrono>

namespace xnet {

class Channel;
class EventLoop;
class TimePoint;

class Poller
{
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    ~Poller() = default;

    TimePoint poll(int timeoutMs, ChannelList* activeChannels);

    void updateChannel(Channel* channel);

private:
    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
};

}

#endif // XNET_POLLER_H

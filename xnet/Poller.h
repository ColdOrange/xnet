//
// Created by Orange on 8/28/17.
//

#ifndef XNET_POLLER_H
#define XNET_POLLER_H

#include <map>
#include <vector>

#include "Noncopyable.h"
#include "Channel.h"
#include "TimePoint.h"

namespace xnet {

class Poller : Noncopyable
{
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop)
        : ownerLoop_(loop)
    { }

    virtual ~Poller() = default;

    virtual TimePoint poll(int timeoutMs, ChannelList* activeChannels) = 0;

    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    ChannelMap channels_;
};

} // namespace xnet

#endif // XNET_POLLER_H

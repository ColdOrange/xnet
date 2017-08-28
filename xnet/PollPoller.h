//
// Created by Orange on 6/24/17.
//

#ifndef XNET_POLLPOLLER_H
#define XNET_POLLPOLLER_H

#include <poll.h>

#include "Poller.h"

namespace xnet {

class EventLoop;

class PollPoller final : public Poller
{
public:
    PollPoller(EventLoop* loop);

    virtual ~PollPoller() override = default;

    virtual TimePoint poll(int timeoutMs, ChannelList* activeChannels) override;

    virtual void updateChannel(Channel* channel) override;
    virtual void removeChannel(Channel* channel) override;

private:
    typedef std::vector<struct pollfd> PollFdList;

    PollFdList pollfds_;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
};

} // namespace xnet

#endif // XNET_POLLPOLLER_H

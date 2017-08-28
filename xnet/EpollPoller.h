//
// Created by Orange on 8/28/17.
//

#ifndef XNET_EPOLLPOLLER_H
#define XNET_EPOLLPOLLER_H

#include <poll.h>
#include <sys/epoll.h>

#include "Poller.h"

namespace xnet {

class EventLoop;

class EpollPoller final : public Poller
{
public:
    typedef std::vector<Channel*> ChannelList;

    EpollPoller(EventLoop* loop);

    virtual ~EpollPoller() override;

    virtual TimePoint poll(int timeoutMs, ChannelList* activeChannels) override;

    virtual void updateChannel(Channel* channel) override;
    virtual void removeChannel(Channel* channel) override;

private:
    static const int kInitialEventListSize = 16;
    typedef std::vector<struct epoll_event> EventList;

    int epollFd_;
    EventList events_;

    static const char* operationToString(int operation);

    void update(int operation, Channel* channel);
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
};

} // namespace xnet

#endif // XNET_EPOLLPOLLER_H

//
// Created by Orange on 8/28/17.
//

#include <assert.h>
#include <stdlib.h>

#include "Poller.h"
#include "PollPoller.h"
#include "EpollPoller.h"
#include "EventLoop.h"

using namespace xnet;

bool Poller::hasChannel(Channel* channel) const
{
    ownerLoop_->assertInLoopThread();
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if (::getenv("XNET_USE_POLL")) {
        return new PollPoller(loop);
    }
    else {
        return new EpollPoller(loop);
    }
}

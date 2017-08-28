//
// Created by Orange on 6/24/17.
//

#include <assert.h>

#include "Poller.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"

using namespace xnet;

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop)
{ }

TimePoint Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::poll(&*pollfds_.data(), static_cast<nfds_t>(pollfds_.size()), timeoutMs);
    TimePoint now(TimePoint::now());

    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0) {
        LOG_TRACE << "nothing happened";
    }
    else {
        LOG_SYSERR << "Poller::poll()";
    }

    return now;
}

void Poller::updateChannel(Channel* channel)
{
    ownerLoop_->assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();

    if (channel->index() < 0) {
        // a new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = static_cast<short>(Channel::kNoneEvent);
        pollfds_.push_back(pfd);
        channel->set_index(static_cast<int>(pollfds_.size()) - 1);
        channels_[pfd.fd] = channel;
    }
    else {
        // update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int index = channel->index();
        assert(0 <= index && index < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[index];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = static_cast<short>(Channel::kNoneEvent);
        if (channel->isNoneEvent()) {
            pfd.fd = -channel->fd()-1; // ignore this pollfd
        }
    }
}

void Poller::removeChannel(Channel* channel)
{
    ownerLoop_->assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(0 <= index && index < static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd = pollfds_[index];
    assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
    channels_.erase(channel->fd());
    if (index == static_cast<int>(pollfds_.size()) - 1) {
        pollfds_.pop_back();
    }
    else {
        int channelToSwap = pollfds_.back().fd; // swap with the last one, to use pop_back()
        if (channelToSwap < 0) {
            channelToSwap = -channelToSwap-1;
        }
        channels_[channelToSwap]->set_index(index);
        iter_swap(pollfds_.begin() + index, pollfds_.end() -1);
        pollfds_.pop_back();
    }
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (const auto& pfd : pollfds_) {
        if (pfd.revents != Channel::kNoneEvent) {
            --numEvents;
            const auto& ch = channels_.find(pfd.fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd.fd);
            channel->set_revents(pfd.revents);
            activeChannels->push_back(channel);
        }
        if (numEvents == 0) {
            break;
        }
    }
}

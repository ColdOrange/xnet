//
// Created by Orange on 6/24/17.
//

#include <assert.h>
#include <iostream>

#include "Poller.h"
#include "Channel.h"
#include "EventLoop.h"

using namespace xnet;

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop)
{ }

Poller::TimePoint Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::poll(&*pollfds_.data(), static_cast<nfds_t>(pollfds_.size()), timeoutMs);
    TimePoint now = std::chrono::system_clock::now();

    if (numEvents > 0) {
        //LOG_TRACE << numEvents << " events happened";
        std::cout << numEvents << " events happened\n";
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0) {
        //LOG_TRACE << "nothing happened";
        std::cout << "nothing happened\n";
    }
    else {
        //LOG_SYSERR << "Poller::poll()";
        std::cout << "Poller::poll() error\n";
        exit(1);
    }

    return now;
}

void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    //LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    std::cout << "fd = " << channel->fd() << " events = " << channel->events() << "\n";

    if (channel->index() < 0) {
        // a new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
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
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            pfd.fd = -1; // ignore this pollfd
        }
    }
}

void Poller::assertInLoopThread()
{
    ownerLoop_->assertInLoopThread();
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (const auto& pfd : pollfds_) {
        if (pfd.revents > 0) {
            numEvents--;
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

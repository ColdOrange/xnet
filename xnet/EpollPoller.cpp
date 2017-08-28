//
// Created by Orange on 8/28/17.
//

#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <iostream>
#include <algorithm>

#include "EpollPoller.h"
#include "EventLoop.h"
#include "Logging.h"

namespace xnet {

namespace detail {

const int kNew = -1; // used for channel.index, which is more suitable to be called state
const int kAdded = 1;
const int kDeleted = 2;

int createEpollFd()
{
    int epollFd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epollFd < 0) {
        LOG_SYSFATAL << "Failed in epoll_create1";
    }
    return epollFd;
}

} // namespace detail

} // namespace xnet

using namespace xnet;
using namespace xnet::detail;

static_assert(EPOLLIN == POLLIN, "The constants of poll() and epoll() are expected to be the same");
static_assert(EPOLLPRI == POLLPRI, "The constants of poll() and epoll() are expected to be the same");
static_assert(EPOLLOUT == POLLOUT, "The constants of poll() and epoll() are expected to be the same");
static_assert(EPOLLHUP == POLLHUP, "The constants of poll() and epoll() are expected to be the same");
static_assert(EPOLLRDHUP == POLLRDHUP, "The constants of poll() and epoll() are expected to be the same");
static_assert(EPOLLERR == POLLERR, "The constants of poll() and epoll() are expected to be the same");

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop),
      epollFd_(createEpollFd()),
      events_(kInitialEventListSize)
{ }

EpollPoller::~EpollPoller()
{
    ::close(epollFd_);
}

TimePoint EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG_TRACE << "fd total count " << channels_.size();
    int numEvents = ::epoll_wait(epollFd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    int savedErrno = errno;
    TimePoint now(TimePoint::now());

    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0) {
        LOG_TRACE << "nothing happened";
    }
    else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_SYSERR << "EpollPoller::poll()";
        }
    }

    return now;
}

void EpollPoller::updateChannel(Channel* channel)
{
    ownerLoop_->assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events() << " index = " << index;

    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        update(EPOLL_CTL_ADD, channel);
        channel->set_index(kAdded);
    }
    else {
        // update existing one with EPOLL_CTL_MOD/DEL
        assert(index == kAdded);
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel)
{
    ownerLoop_->assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    assert(n == 1);
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

const char* EpollPoller::operationToString(int operation)
{
    switch (operation) {
        case EPOLL_CTL_ADD: return "ADD";
        case EPOLL_CTL_DEL: return "DEL";
        case EPOLL_CTL_MOD: return "MOD";
        default:
            assert(false);
            return "Unknown Operation";
    }
}

void EpollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd
              << " event = { " << channel->eventsToString() << " }";
    if (::epoll_ctl(epollFd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd;
        }
        else {
            LOG_SYSFATAL << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd;
        }
    }
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        auto it = channels_.find(channel->fd());
        assert(it != channels_.end() && it->second == channel);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

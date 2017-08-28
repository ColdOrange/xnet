//
// Created by Orange on 6/24/17.
//

#include <poll.h>
#include <assert.h>
#include <sstream>

#include "EventLoop.h"
#include "Channel.h"
#include "Logging.h"

using namespace xnet;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : ownerLoop_(loop),
      fd_(fd),
      events_(kNoneEvent),
      revents_(kNoneEvent),
      index_(-1),
      eventHandling_(false)
{ }

Channel::~Channel()
{
    assert(!eventHandling_);
}

void Channel::handleEvent(TimePoint receiveTime)
{
    eventHandling_ = true;
    if (revents_ & POLLNVAL) {
        LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) readCallback_(receiveTime);
    }
    if (revents_ & POLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}

std::string Channel::eventsToString() const
{
    return eventsToString(fd_, events_);
}

std::string Channel::reventsToString() const
{
    return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString(int fd, int event)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (event & POLLIN) oss << "IN ";
    if (event & POLLPRI) oss << "PRI ";
    if (event & POLLOUT) oss << "OUT ";
    if (event & POLLHUP) oss << "HUP ";
    if (event & POLLRDHUP) oss << "RDHUP ";
    if (event & POLLERR) oss << "ERR ";
    if (event & POLLNVAL) oss << "NVAL ";
    return oss.str();
}

void Channel::update()
{
    ownerLoop_->updateChannel(this);
}

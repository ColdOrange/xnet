//
// Created by Orange on 6/24/17.
//

#include <poll.h>
#include <assert.h>
#include <iostream>

#include "Channel.h"
#include "EventLoop.h"

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

void Channel::handleEvent(const TimePoint& receiveTime)
{
    eventHandling_ = true;
    if (revents_ & POLLNVAL) {
        //LOG_WARN << "Channel::handle_event() POLLNVAL";
        std::cout << "Channel::handle_event() POLLNVAL\n";
    }

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        //LOG_WARN << "Channel::handle_event() POLLHUP";
        std::cout << "Channel::handle_event() POLLHUP\n";
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI)) {
        if (readCallback_) readCallback_(receiveTime);
    }
    if (revents_ & POLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}

void Channel::update()
{
    ownerLoop_->updateChannel(this);
}

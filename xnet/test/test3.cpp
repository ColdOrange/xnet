//
// Created by Orange on 6/25/17.
//

#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/timerfd.h>

#include "../Channel.h"
#include "../EventLoop.h"

xnet::EventLoop* g_eventLoop;

void timeout(const xnet::TimePoint& receiveTime)
{
    printf("%s timeout!\n", receiveTime.toFormattedString().c_str());
    g_eventLoop->quit();
}

int main()
{
    printf("%s started.\n", xnet::TimePoint::now().toFormattedString().c_str());
    xnet::EventLoop eventLoop;
    g_eventLoop = &eventLoop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    xnet::Channel channel(&eventLoop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    eventLoop.loop();

    ::close(timerfd);
}
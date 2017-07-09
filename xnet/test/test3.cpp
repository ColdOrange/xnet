//
// Created by Orange on 6/25/17.
//

#include <stdio.h>

#include "../Channel.h"
#include "../EventLoop.h"

xnet::EventLoop* g_eventLoop;

void done(const xnet::TimePoint& receiveTime)
{
    printf("%s done!\n", receiveTime.toFormattedString().c_str());
    g_eventLoop->quit();
}

int main()
{
    printf("%s started.\n", xnet::TimePoint::now().toFormattedString().c_str());
    xnet::EventLoop eventLoop;
    g_eventLoop = &eventLoop;

    xnet::Channel channel(&eventLoop, fileno(stdin));
    channel.setReadCallback(done);
    channel.enableReading();

    eventLoop.loop();
}
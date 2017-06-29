//
// Created by Orange on 6/25/17.
//

#include <stdio.h>

#include "../Channel.h"
#include "../EventLoop.h"

xnet::EventLoop* g_eventLoop;

void done()
{
    printf("Done!\n");
    g_eventLoop->quit();
}

int main()
{
    xnet::EventLoop eventLoop;
    g_eventLoop = &eventLoop;

    xnet::Channel channel(&eventLoop, fileno(stdin));
    channel.setReadCallback(done);
    channel.enableReading();

    eventLoop.loop();
}
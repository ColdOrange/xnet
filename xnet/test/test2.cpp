//
// Created by Orange on 6/23/17.
//

//
// Created by Orange on 6/23/17.
//

#include <stdio.h>
#include <unistd.h>
#include "../EventLoop.h"

xnet::EventLoop* g_eventLoop;

void threadFunc()
{
    g_eventLoop->loop();
}

int main()
{
    xnet::EventLoop eventLoop;
    g_eventLoop = &eventLoop;

    std::thread thread(threadFunc);
    thread.join();
}
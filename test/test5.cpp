//
// Created by Orange on 6/28/17.
//

#include <stdio.h>
#include <unistd.h>

#include "../EventLoop.h"

xnet::EventLoop* g_eventLoop;
int g_flag = 0;

void run4()
{
    printf("run4(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_eventLoop->quit();
}

void run3()
{
    printf("run3(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_eventLoop->runAfter(3, run4);
    g_flag = 3;
}

void run2()
{
    printf("run2(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_eventLoop->queueInLoop(run3);
}

void run1()
{
    g_flag = 1;
    printf("run1(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_eventLoop->runInLoop(run2);
    g_flag = 2;
}

int main()
{
    printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);

    xnet::EventLoop eventLoop;
    g_eventLoop = &eventLoop;

    eventLoop.runAfter(2, run1);
    eventLoop.loop();
    printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);
}

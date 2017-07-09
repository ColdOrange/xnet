//
// Created by Orange on 6/26/17.
//

#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include "../EventLoop.h"

int cnt = 0;
xnet::EventLoop* g_eventLoop;

void printTid()
{
    std::cout << "pid = " << getpid() << ", tid = " << std::this_thread::get_id() << "\n";
    printf("now %s\n", xnet::TimePoint::now().toFormattedString().c_str());
}

void print(const char* msg)
{
    printf("msg %s %s\n", xnet::TimePoint::now().toFormattedString().c_str(), msg);
    if (++cnt == 20) {
        g_eventLoop->quit();
    }
}

int main()
{
    printTid();
    xnet::EventLoop eventLoop;
    g_eventLoop = &eventLoop;

    print("main");
    eventLoop.runAfter(  1, [] { print("once1"); });
    eventLoop.runAfter(1.5, [] { print("once1.5"); });
    eventLoop.runAfter(2.5, [] { print("once2.5"); });
    eventLoop.runAfter(3.5, [] { print("once3.5"); });
    eventLoop.runEvery(  2, [] { print("every2"); });
    eventLoop.runEvery(  3, [] { print("every3"); });

    eventLoop.loop();
    print("main loop exits");
    sleep(1);
}
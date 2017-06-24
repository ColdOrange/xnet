//
// Created by Orange on 6/23/17.
//

#include <stdio.h>
#include <unistd.h>
#include "../EventLoop.h"

void threadFunc()
{
    printf("threadFunc(): pid = %d, tid = %p\n", getpid(), std::this_thread::get_id());

    xnet::EventLoop eventLoop;
    eventLoop.loop();

    printf("threadFunc() died\n");
}

int main()
{
    printf("main(): pid = %d, tid = %p\n", getpid(), std::this_thread::get_id());

    xnet::EventLoop eventLoop;

    std::thread thread(threadFunc);
    thread.detach();

    eventLoop.loop();

    printf("main() died\n");
    pthread_exit(nullptr);
}
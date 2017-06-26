//
// Created by Orange on 6/23/17.
//

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "../EventLoop.h"

void threadFunc()
{
    std::cout << "threadFunc(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << "\n";

    xnet::EventLoop eventLoop;
    eventLoop.loop();

    printf("threadFunc() died\n");
}

int main()
{
    std::cout << "main(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << "\n";

    xnet::EventLoop eventLoop;

    std::thread thread(threadFunc);
    thread.detach();

    eventLoop.loop();

    printf("main() died\n");
    pthread_exit(nullptr);
}
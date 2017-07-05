//
// Created by Orange on 6/29/17.
//

#include <unistd.h>
#include <iostream>

#include "../EventLoop.h"
#include "../EventLoopThread.h"

void runInThread()
{
    std::cout << "runInThread(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << "\n";
}

int main()
{
    std::cout << "main(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << "\n";

    xnet::EventLoopThread eventLoopThread;
    xnet::EventLoop* eventLoop = eventLoopThread.startLoop(); // Run in another thread

    eventLoop->runInLoop(runInThread);
    sleep(1);
    eventLoop->runAfter(2, runInThread);
    sleep(3);
    eventLoop->quit();
    printf("main() died\n");
}
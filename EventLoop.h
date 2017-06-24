//
// Created by Orange on 6/23/17.
//

#ifndef XNET_EVENTLOOP_H
#define XNET_EVENTLOOP_H

#include <thread>

namespace xnet {

class EventLoop
{
public:
    EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    virtual ~EventLoop();

    void loop();

    void assertInLoopThread();
    bool isInLoopThread() const;

private:
    bool looping_;
    std::thread::id threadId_;

    void abortNotInLoopThread();
};

} // namespace xnet

#endif //XNET_EVENTLOOP_H

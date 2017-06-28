//
// Created by Orange on 6/24/17.
//

#ifndef XNET_CHANNEL_H
#define XNET_CHANNEL_H

#include <functional>

namespace xnet {

class EventLoop;

class Channel
{
public:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    typedef std::function<void()> EventCallback;

    Channel(EventLoop* loop, int fd);

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    void handleEvent();

    void setReadCallback(const EventCallback& cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }

    EventLoop* ownerLoop() { return ownerLoop_; }
    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revents) { revents_ = revents; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    //void enableWriting() { events_ |= kWriteEvent; update(); }
    //void disableWriting() { events_ &= ~kWriteEvent; update(); }
    //void disableAll() { events_ = kNoneEvent; update(); }

    // for Poller
    int index() const { return index_; }
    void set_index(int index) { index_ = index; }

private:
    EventLoop* ownerLoop_;
    const int  fd_;
    int        events_;
    int        revents_;
    int        index_; // used by Poller

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;

    void update();
};

} // namespace xnet

#endif // XNET_CHANNEL_H

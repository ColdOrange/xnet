//
// Created by Orange on 6/24/17.
//

#ifndef XNET_CHANNEL_H
#define XNET_CHANNEL_H

#include <string>
#include <functional>

#include "Noncopyable.h"
#include "TimePoint.h"

namespace xnet {

class EventLoop;

class Channel : Noncopyable
{
public:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    typedef std::function<void()> EventCallback;
    typedef std::function<void(TimePoint)> ReadEventCallback;

    Channel(EventLoop* loop, int fd);

    ~Channel();

    void handleEvent(TimePoint receiveTime);

    void setReadCallback(const ReadEventCallback& cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }

    EventLoop* ownerLoop() { return ownerLoop_; }
    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revents) { revents_ = revents; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    bool isReading() const { return events_ & kReadEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }

    int index() const { return index_; }
    void set_index(int index) { index_ = index; }

    std::string eventsToString() const;
    std::string reventsToString() const;

private:
    EventLoop* ownerLoop_;
    const int fd_;
    int events_;
    int revents_;
    int index_; // used by Poller
    bool eventHandling_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    static std::string eventsToString(int fd, int event);
    void update();
};

} // namespace xnet

#endif // XNET_CHANNEL_H

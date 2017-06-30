//
// Created by Orange on 6/30/17.
//

#ifndef XNET_ACCEPTOR_H
#define XNET_ACCEPTOR_H

#include <functional>

#include "Channel.h"
#include "Socket.h"

namespace xnet {

class EventLoop;
class InetAddress;

class Acceptor
{
public:
    typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* eventLoop, const InetAddress& listenAddress);

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    void setNewConnectionCallback(const NewConnectionCallback& cb);

    void listen();
    bool listening() const { return listening_; }

private:
    EventLoop* eventLoop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;

    void handleRead();
};

}

#endif // XNET_ACCEPTOR_H

//
// Created by Orange on 6/30/17.
//

#ifndef XNET_ACCEPTOR_H
#define XNET_ACCEPTOR_H

#include <functional>

#include "Noncopyable.h"
#include "Channel.h"
#include "Socket.h"
#include "EventLoop.h"
#include "InetAddress.h"

namespace xnet {

class Acceptor : Noncopyable
{
public:
    typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* eventLoop, const InetAddress& listenAddress);

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

} // namespace xnet

#endif // XNET_ACCEPTOR_H

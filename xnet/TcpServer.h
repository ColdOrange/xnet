//
// Created by Orange on 7/6/17.
//

#ifndef XNET_TCPSERVER_H
#define XNET_TCPSERVER_H

#include <map>
#include <string>
#include <memory>

#include "Acceptor.h"
#include "Callbacks.h"
#include "TcpConnection.h"

namespace xnet {

class EventLoop;
class InetAddress;

class TcpServer
{
public:
    TcpServer(EventLoop* eventLoop, const InetAddress& listenAddress);

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    void start();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

private:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* eventLoop_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    bool started_;
    int nextConnId_;
    ConnectionMap connections_;

    void newConnection(int sockfd, const InetAddress& peerAddress);
};

} // namespace xnet

#endif // XNET_TCPSERVER_H

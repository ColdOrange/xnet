//
// Created by Orange on 7/6/17.
//

#include <stdio.h>
#include <iostream>

#include "TcpServer.h"
#include "SocketOps.h"
#include "EventLoop.h"
#include "InetAddress.h"

using namespace xnet;

TcpServer::TcpServer(EventLoop* eventLoop, const InetAddress& listenAddress)
    : eventLoop_(eventLoop),
      name_(listenAddress.toHostPort()),
      acceptor_(new Acceptor(eventLoop, listenAddress)),
      started_(false),
      nextConnId_(1)
{
    acceptor_->setNewConnectionCallback([this](int sockfd, const InetAddress& peerAddress) {
        newConnection(sockfd, peerAddress);
    });
}

void TcpServer::start()
{
    if (!started_) {
        started_ = true;
    }
    if (!acceptor_->listening()) {
        eventLoop_->runInLoop([this] { acceptor_->listen(); });
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddress)
{
    eventLoop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", nextConnId_);
    ++nextConnId_;
    std::string connectionName = name_ + buf;
    //LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
    //         << connName << "] from " << peerAddress.toHostPort();
    std::cout << "TcpServer::newConnection [" << name_ << "] - new connection ["
              << connectionName << "] from " << peerAddress.toHostPort();

    InetAddress localAddress(sockops::getLocalAddress(sockfd));
    TcpConnectionPtr connection(
        new TcpConnection(eventLoop_, connectionName, sockfd, localAddress, peerAddress));
    connections_[connectionName] = connection;
    connection->setConnectionCallback(connectionCallback_);
    connection->setMessageCallback(messageCallback_);
    connection->connectEstablished();
}

//
// Created by Orange on 7/6/17.
//

#include <assert.h>
#include <unistd.h>
#include <iostream>

#include "EventLoop.h"
#include "TcpConnection.h"

using namespace xnet;

TcpConnection::TcpConnection(EventLoop* eventLoop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& localAddress,
                             const InetAddress& peerAddress)
    : eventLoop_(eventLoop),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(eventLoop_, sockfd)),
      localAddress_(localAddress),
      peerAddress_(peerAddress)
{
    //LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this << " fd=" << sockfd;
    std::cout << "TcpConnection::ctor[" <<  name_ << "] at " << this << " fd=" << sockfd << "\n";
    channel_->setReadCallback([this] { handleRead(); });
}

TcpConnection::~TcpConnection()
{
    //LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this << " fd=" << channel_->fd();
    std::cout << "TcpConnection::dtor[" <<  name_ << "] at " << this << " fd=" << channel_->fd() << "\n";
}

void TcpConnection::connectEstablished()
{
    eventLoop_->assertInLoopThread();
    assert(state_ == kConnecting);
    state_ = kConnected;
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead()
{
    char buf[65536];
    ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
    messageCallback_(shared_from_this(), buf, n);
}

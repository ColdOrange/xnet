//
// Created by Orange on 7/6/17.
//

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>

#include "EventLoop.h"
#include "SocketOps.h"
#include "TcpConnection.h"

namespace xnet {

namespace tmp { // TODO: Move strerror_tl to Log Utils

__thread char t_errnobuf[512];

const char* strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

} // namespace tmp

} // namespace xnet

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
    channel_->setReadCallback([this](const TimePoint& receiveTime) {
        handleRead(receiveTime);
    });
    channel_->setWriteCallback([this] { handleWrite(); });
    channel_->setCloseCallback([this] { handleClose(); });
    channel_->setErrorCallback([this] { handleError(); });
}

TcpConnection::~TcpConnection()
{
    //LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this << " fd=" << channel_->fd();
    std::cout << "TcpConnection::dtor[" <<  name_ << "] at " << this << " fd=" << channel_->fd() << "\n";
}

void TcpConnection::connectionEstablished()
{
    eventLoop_->assertInLoopThread();
    assert(state_ == kConnecting);
    state_ = kConnected;
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectionDestroyed()
{
    eventLoop_->assertInLoopThread();
    assert(state_ == kConnected); // TODO: Improve this assert
    state_ = kDisconnected;
    channel_->disableAll();
    connectionCallback_(shared_from_this());
    eventLoop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead(const TimePoint& receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0) {
        handleClose();
    }
    else {
        errno = savedErrno;
        //LOG_SYSERR << "TcpConnection::handleRead";
        std::cout << "TcpConnection::handleRead\n";
        handleError();
    }
}

void TcpConnection::handleWrite()
{

}

void TcpConnection::handleClose()
{
    eventLoop_->assertInLoopThread();
    //LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected);
    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = sockops::getSocketError(channel_->fd());
    //LOG_ERROR << "TcpConnection::handleError [" << name_
    //          << "] - SO_ERROR = " << err << " " << strerror_tl(err);
    std::cout << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << tmp::strerror_tl(err);
}

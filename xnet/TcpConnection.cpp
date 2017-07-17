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
    int ret = strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
    assert(ret == 0);
    return t_errnobuf;
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

void TcpConnection::send(const std::string& message)
{
    if (state_ == kConnected) {
        eventLoop_->runInLoop([this, &message] { sendInLoop(message); }); // TODO: shared_from_this()?
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected) {
        state_ = kDisconnecting;
        eventLoop_->runInLoop([this] { shutdownInLoop(); }); // TODO: shared_from_this()?
    }
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
    assert(state_ == kConnected || state_ == kDisconnecting); // TODO: Improve this assert
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
    eventLoop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t nwritten = ::write(channel_->fd(),
                                   outputBuffer_.readerBegin(), outputBuffer_.readableBytes());
        if (nwritten > 0) {
            outputBuffer_.retrieve(static_cast<size_t>(nwritten));
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
            else {
                //LOG_TRACE << "I am going to write more data";
            }
        }
        else {
            //LOG_SYSERR << "TcpConnection::handleWrite";
            std::cout << "TcpConnection::handleWrite\n";
        }
    }
    else {
        //LOG_TRACE << "Connection is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    eventLoop_->assertInLoopThread();
    //LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected || state_ == kDisconnecting);
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

void TcpConnection::sendInLoop(const std::string& message)
{
    eventLoop_->assertInLoopThread();
    ssize_t nwritten = 0;
    // If no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwritten = ::write(channel_->fd(), message.data(), message.size());
        if (nwritten >= 0) {
            if (static_cast<size_t>(nwritten) < message.size()) {
                //LOG_TRACE << "I am going to write more data";
                //std::cout << "I am going to write more data\n";
            }
        } else {
            nwritten = 0;
            if (errno != EWOULDBLOCK) {
                //LOG_SYSERR << "TcpConnection::sendInLoop";
                std::cout << "TcpConnection::sendInLoop\n";
            }
        }
    }

    assert(nwritten >= 0);
    if (static_cast<size_t>(nwritten) < message.size()) {
        outputBuffer_.append(message.data() + nwritten, message.size() - nwritten);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    eventLoop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

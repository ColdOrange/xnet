//
// Created by Orange on 6/30/17.
//

#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketOps.h"

using namespace xnet;

Acceptor::Acceptor(EventLoop* eventLoop, const InetAddress& listenAddress)
    : eventLoop_(eventLoop),
      acceptSocket_(sockops::createNonblockingOrDie()),
      acceptChannel_(eventLoop_, acceptSocket_.fd()),
      listening_(false)
{
    acceptSocket_.setReuseAddress(true);
    acceptSocket_.bindAddress(listenAddress);
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb)
{
    newConnectionCallback_ = cb;
}

void Acceptor::listen()
{
    eventLoop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.setReadCallback([this]{ handleRead(); });
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    eventLoop_->assertInLoopThread();
    InetAddress peerAddress(0);
    int connfd = acceptSocket_.accept(&peerAddress);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddress);
        }
        else {
            sockops::close(connfd);
        }
    }
}

//
// Created by Orange on 7/6/17.
//

#include <stdio.h>

#include "TcpServer.h"
#include "SocketOps.h"
#include "Logging.h"

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
    LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
             << connectionName << "] from " << peerAddress.toHostPort();

    InetAddress localAddress(sockops::getLocalAddress(sockfd));
    TcpConnectionPtr connection(
        new TcpConnection(eventLoop_, connectionName, sockfd, localAddress, peerAddress));
    connections_[connectionName] = connection;
    connection->setConnectionCallback(connectionCallback_);
    connection->setMessageCallback(messageCallback_);
    connection->setCloseCallback([this](const TcpConnectionPtr& conn) {
        removeConnection(conn);
    });
    connection->connectionEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& connection)
{
    eventLoop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnection [" << name_ << "] - connection " << connection->name();

    connections_.erase(connection->name());

    // Why not just call connection->connectionDestroyed()?
    //
    // It is for extending connection's lifetime:
    // After connections_.erase(), the only one reference is the connection variable
    // which comes from shared_from_this() in TcpConnection::handleClose(). We should
    // bind a reference to connection->connectionDestroyed() to extend connection's
    // lifetime using lambda or std::bind. Because Channel must destruct before
    // TcpConnection, but without one more reference, Channel will be immediately
    // destructed after doing closeCallback, while it has more work to be done.
    // Using queueInLoop, we can extend connection's lifetime till channel->handleEvent()
    // is done, when it is safe to destruct this channel.
    //
    // Note the capture of connection in lambda must be by value. Or use
    // eventLoop_->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, connection));
    eventLoop_->queueInLoop([connection] { connection->connectionDestroyed(); });
}

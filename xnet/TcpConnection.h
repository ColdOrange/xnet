//
// Created by Orange on 7/6/17.
//

#ifndef XNET_TCPCONNECTION_H
#define XNET_TCPCONNECTION_H

#include <string>
#include <memory>

#include "Buffer.h"
#include "Channel.h"
#include "Socket.h"
#include "Callbacks.h"
#include "InetAddress.h"

namespace xnet {

class EventLoop;

//
// TCP connection, for both client and server usage.
//
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    // Constructs a TcpConnection with a connected sockfd
    //
    // User should not create this object.
    TcpConnection(EventLoop* eventLoop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& localAddress,
                  const InetAddress& peerAddress);

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    ~TcpConnection();

    EventLoop* getEventLoop() const { return eventLoop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() { return localAddress_; }
    const InetAddress& peerAddress() { return peerAddress_; }
    bool connected() const { return state_ == kConnected; }

    void send(const void* message, size_t len); // TODO: implement this
    void send(const std::string& message);
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void connectionEstablished();
    void connectionDestroyed();

private:
    enum State { kConnecting, kConnected, kDisconnecting, kDisconnected };

    EventLoop* eventLoop_;
    std::string name_;
    State state_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress localAddress_;
    InetAddress peerAddress_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    void handleRead(const TimePoint& receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const std::string& message);
    void shutdownInLoop();
};

} // namespace xnet

#endif // XNET_TCPCONNECTION_H

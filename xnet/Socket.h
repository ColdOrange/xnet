//
// Created by Orange on 6/30/17.
//

#ifndef XNET_SOCKET_H
#define XNET_SOCKET_H

#include "Noncopyable.h"
#include "InetAddress.h"

namespace xnet {

class Socket : Noncopyable
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    { }

    ~Socket();

    int fd() const { return sockfd_; }

    void bindAddress(const InetAddress& localAddress);
    void listen();
    int accept(InetAddress* peerAddress);
    void setReuseAddress(bool on);
    void shutdownWrite();

private:
    const int sockfd_;
};

} // namespace xnet

#endif // XNET_SOCKET_H

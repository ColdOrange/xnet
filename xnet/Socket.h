//
// Created by Orange on 6/30/17.
//

#ifndef XNET_SOCKET_H
#define XNET_SOCKET_H

namespace xnet {

class InetAddress;

class Socket
{
public:
    explicit Socket(int sockfd);

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

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

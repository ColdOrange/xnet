//
// Created by Orange on 6/30/17.
//

#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Socket.h"
#include "SocketOps.h"
#include "Logging.h"

using namespace xnet;

Socket::~Socket()
{
    sockops::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localAddress)
{
    sockops::bindOrDie(sockfd_, localAddress.getSockAddrInet());
}

void Socket::listen()
{
    sockops::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peerAddress)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    int connfd = sockops::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peerAddress->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::setReuseAddress(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret < 0) {
        LOG_SYSERR << "Socket::setReuseAddress";
    }
}

void Socket::shutdownWrite()
{
    sockops::shutdownWrite(sockfd_);
}

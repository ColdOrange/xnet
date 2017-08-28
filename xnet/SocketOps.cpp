//
// Created by Orange on 6/29/17.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <sys/socket.h>
#include <iostream>

#include "SocketOps.h"
#include "Logging.h"

namespace xnet {

namespace detail {

void setNonBlockAndCloseOnExec(int sockfd)
{
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    if (flags == -1 || ::fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_SYSFATAL << "fcntl::setNonblock";
    }

    flags = ::fcntl(sockfd, F_GETFD, 0);
    if (flags == -1 || ::fcntl(sockfd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        LOG_SYSFATAL << "fcntl::setCloseOnExec";
    }
}

} // namespace detail

} // namespace xnet

using namespace xnet;
using namespace xnet::detail;

int sockops::createNonblockingOrDie()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    setNonBlockAndCloseOnExec(sockfd);
    return sockfd;
}

void sockops::bindOrDie(int sockfd, const struct sockaddr_in& addr)
{
    int ret = ::bind(sockfd, reinterpret_cast<struct sockaddr*>(const_cast<struct sockaddr_in*>(&addr)), sizeof(addr));
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::bindOrDie";
    }
}

void sockops::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::listenOrDie";
    }
}

int sockops::accept(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = sizeof(*addr);
    int connfd = ::accept(sockfd, reinterpret_cast<struct sockaddr*>(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);

    if (connfd < 0) {
        int savedErrno = errno;
        LOG_SYSERR << "sockops::accept";
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    return connfd;
}

void sockops::close(int sockfd)
{
    if (::close(sockfd) < 0) {
        LOG_SYSERR << "sockops::close";
    }
}

void sockops::shutdownWrite(int sockfd)
{
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        LOG_SYSERR << "sockops::shutdownWrite";
    }
}

void sockops::toHostPort(char* buf, size_t size, const struct sockaddr_in& addr)
{
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host));
    uint16_t port = sockops::networkToHost16(addr.sin_port);
    snprintf(buf, size, "%s:%u", host, port);
}

void sockops::fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) { //TODO: Distinguish 2 kinds of errors (0 or <0)
        LOG_SYSERR << "sockops::fromHostPort";
    }
}

struct sockaddr_in sockops::getLocalAddress(int sockfd)
{
    struct sockaddr_in localAddress;
    bzero(&localAddress, sizeof(localAddress));
    socklen_t addressLen = sizeof(localAddress);
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localAddress), &addressLen) < 0) {
        LOG_SYSERR << "sockops::getLocalAddress";
    }
    return localAddress;
}

struct sockaddr_in sockops::getPeerAddress(int sockfd)
{
    struct sockaddr_in peerAddress;
    bzero(&peerAddress, sizeof(peerAddress));
    socklen_t addressLen = sizeof(peerAddress);
    if (::getpeername(sockfd, reinterpret_cast<struct sockaddr*>(&peerAddress), &addressLen) < 0) {
        LOG_SYSERR << "sockops::getPeerAddress";
    }
    return peerAddress;
}

int sockops::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = sizeof(optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    }
    else {
        return optval;
    }
}

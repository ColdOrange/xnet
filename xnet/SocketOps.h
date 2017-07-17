//
// Created by Orange on 6/29/17.
//

#ifndef XNET_SOCKETOPS_H
#define XNET_SOCKETOPS_H

#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <libkern/OSByteOrder.h>

namespace xnet {

namespace sockops {

inline uint16_t hostToNetwork16(uint16_t host16)
{
    return htons(host16);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
    return htonl(host32);
}

inline uint64_t hostToNetwork64(uint64_t host64)
{
    return OSSwapHostToBigInt64(host64);
}

inline uint16_t networkToHost16(uint16_t net16)
{
    return ntohs(net16);
}

inline uint32_t networkToHost32(uint32_t net32)
{
    return ntohl(net32);
}

inline uint64_t networkToHost64(uint64_t net64)
{
    return OSSwapBigToHostInt64(net64);
}

// Create a non-blocking sockfd, or abort on error.
int createNonblockingOrDie();

void bindOrDie(int sockfd, const struct sockaddr_in& addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toHostPort(char* buf, size_t size, const struct sockaddr_in& addr);
void fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

struct sockaddr_in getLocalAddress(int sockfd);
struct sockaddr_in getPeerAddress(int sockfd);

int getSocketError(int sockfd);

} // namespace socket

} // namespace xnet

#endif // XNET_SOCKETOPS_H

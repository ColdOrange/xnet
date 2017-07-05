//
// Created by Orange on 6/30/17.
//

#ifndef XNET_INETADDRESS_H
#define XNET_INETADDRESS_H

#include <stdint.h>
#include <netinet/in.h>
#include <string>

namespace xnet {

class InetAddress
{
public:
    explicit InetAddress(uint16_t port);
    InetAddress(const std::string& ip, uint16_t port);
    InetAddress(const struct sockaddr_in& addr);

    InetAddress(const InetAddress&) = default;
    InetAddress& operator=(const InetAddress&) = default;

    std::string toHostPort() const;

    const struct sockaddr_in& getSockAddrInet() const { return addr_; }
    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

private:
    struct sockaddr_in addr_;
};

} // namespace xnet

#endif // XNET_INETADDRESS_H

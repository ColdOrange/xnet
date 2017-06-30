//
// Created by Orange on 6/30/17.
//

#include <strings.h>

#include "InetAddress.h"
#include "SocketOps.h"

using namespace xnet;

InetAddress::InetAddress(uint16_t port)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = sockops::hostToNetwork32(INADDR_ANY);
    addr_.sin_port = sockops::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    bzero(&addr_, sizeof(addr_));
    sockops::fromHostPort(ip.c_str(), port, &addr_);
}

InetAddress::InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
{ }

std::string InetAddress::toHostPort() const
{
    char buf[32];
    sockops::toHostPort(buf, sizeof(buf), addr_);
    return buf;
}
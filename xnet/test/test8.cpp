//
// Created by Orange on 7/6/17.
//

#include <stdio.h>
#include <unistd.h>

#include "../TcpServer.h"
#include "../EventLoop.h"
#include "../InetAddress.h"

void onConnection(const xnet::TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
    }
    else {
        printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
    }
}

void onMessage(const xnet::TcpConnectionPtr& conn, const char* buf, ssize_t len)
{
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           len, conn->name().c_str());
}

int main()
{
    printf("main(): pid = %d\n", getpid());

    xnet::InetAddress listenAddress(9877);
    xnet::EventLoop eventLoop;

    xnet::TcpServer server(&eventLoop, listenAddress);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    eventLoop.loop();
}
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

void onMessage(const xnet::TcpConnectionPtr& conn, xnet::Buffer* buf, xnet::TimePoint receiveTime)
{
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->readableBytes(), conn->name().c_str(), receiveTime.toFormattedString().c_str());

    printf("onMessage(): [%s]\n", buf->retrieveAllAsString().c_str());
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

//
// Created by Orange on 6/30/17.
//

#include <stdio.h>
#include <unistd.h>

#include "../Acceptor.h"
#include "../EventLoop.h"
#include "../InetAddress.h"
#include "../SocketOps.h"

void newConnection(int sockfd, const xnet::InetAddress& peerAddress)
{
    printf("newConnection(): accepted a new connection from %s\n", peerAddress.toHostPort().c_str());
    ::write(sockfd, "How are you?\n", 13);
    xnet::sockops::close(sockfd);
}

int main()
{
    printf("main(): pid = %d\n", getpid());

    xnet::InetAddress listenAddress(9877);
    xnet::EventLoop eventLoop;

    xnet::Acceptor acceptor(&eventLoop, listenAddress);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    eventLoop.loop();
}


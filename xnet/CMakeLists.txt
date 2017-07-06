set(SOURCE_FILES
    EventLoop.cpp EventLoop.h
    Channel.cpp Channel.h
    Poller.cpp Poller.h
    Callbacks.h
    TimePoint.cpp TimePoint.h
    Timer.cpp Timer.h
    TimerId.h
    TimerQueue.cpp TimerQueue.h
    EventLoopThread.cpp EventLoopThread.h
    SocketOps.cpp SocketOps.h
    Socket.cpp Socket.h
    InetAddress.cpp InetAddress.h
    Acceptor.cpp Acceptor.h
    TcpConnection.cpp TcpConnection.h
    TcpServer.cpp TcpServer.h)

add_library(xnet ${SOURCE_FILES})

add_subdirectory(test)

set(SOURCE_FILES
        Acceptor.cpp
        AsyncLogging.cpp
        Buffer.cpp
        Channel.cpp
        EpollPoller.cpp
        EventLoop.cpp
        EventLoopThread.cpp
        InetAddress.cpp
        LogFile.cpp
        Logging.cpp
        LogStream.cpp
        Poller.cpp
        PollPoller.cpp
        Socket.cpp
        SocketOps.cpp
        TcpConnection.cpp
        TcpServer.cpp
        TimePoint.cpp
        Timer.cpp
        TimerQueue.cpp)

add_library(xnet ${SOURCE_FILES})

add_subdirectory(test)

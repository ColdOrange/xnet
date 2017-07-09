//
// Created by Orange on 6/25/17.
//

#ifndef XNET_CALLBACKS_H
#define XNET_CALLBACKS_H

#include <memory>
#include <functional>

namespace xnet {

// Client visible callbacks

class Buffer;
class TimePoint;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> TimerCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer*, const TimePoint&)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;

}

#endif // XNET_CALLBACKS_H

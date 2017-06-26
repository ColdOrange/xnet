//
// Created by Orange on 6/25/17.
//

#ifndef XNET_CALLBACKS_H
#define XNET_CALLBACKS_H

#include <functional>

namespace xnet {

// Client visible callbacks

typedef std::function<void()> TimerCallback;

}

#endif // XNET_CALLBACKS_H

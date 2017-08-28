//
// Created by Orange on 8/27/17.
//

#ifndef XNET_NONCOPYABLE_H
#define XNET_NONCOPYABLE_H

namespace xnet {

//
// Base class for classes that cannot be copied, taken from boost::noncopyable.
//
class Noncopyable
{
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};

} // namespace xnet

#endif // XNET_NONCOPYABLE_H

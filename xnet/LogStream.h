//
// Created by Orange on 8/27/17.
//

#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include <assert.h>
#include <string.h>
#include <strings.h>
#include <string>
#include <thread>

#include "Copyable.h"
#include "Noncopyable.h"

namespace xnet {

namespace detail {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : Noncopyable
{
public:
    FixedBuffer()
        : cur_(data_)
    {}

    void append(const char* data, size_t len)
    {
        if (available() > len) {
            ::memcpy(cur_, data, len);
            cur_ += len;
        }
    }

    const char* data() const
    { return data_; }

    size_t length() const
    { return cur_ - data_; }

    // Write to data_ directly
    char* current()
    { return cur_; }

    size_t available() const
    { return data_ + sizeof(data_) - cur_; }

    void add(size_t len)
    { cur_ += len; }

    void reset()
    { cur_ = data_; }

    void bzero()
    { ::bzero(data_, sizeof(data_)); }

private:
    char data_[SIZE];
    char* cur_;
};

} // namespace detail

// Helper class for known string length at compile time
class T : public Copyable
{
public:
    T(const char* str, size_t len)
        : str_(str), len_(len)
    {
        assert(strlen(str) == len_);
    }

    const char* str_;
    const size_t len_;
};

class LogStream : Noncopyable
{
    typedef LogStream self;

public:
    typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

    self& operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<<(short);

    self& operator<<(unsigned short);

    self& operator<<(int);

    self& operator<<(unsigned int);

    self& operator<<(long);

    self& operator<<(unsigned long);

    self& operator<<(long long);

    self& operator<<(unsigned long long);

    self& operator<<(const void*);

    self& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }

    self& operator<<(double);
    //self& operator<<(long double);

    self& operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }
    //self& operator<<(signed char);
    //self& operator<<(unsigned char);

    self& operator<<(const char* v)
    {
        if (v) {
            buffer_.append(v, strlen(v));
        } else {
            buffer_.append("(null)", 6);
        }
        return *this;
    }

    self& operator<<(const T& v)
    {
        buffer_.append(v.str_, v.len_);
        return *this;
    }

    self& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    void append(const char* data, size_t len)
    { buffer_.append(data, len); }

    const Buffer& buffer() const
    { return buffer_; }

    void resetBuffer()
    { buffer_.reset(); }

private:
    static const int kMaxNumericSize = 32;

    Buffer buffer_;

    void staticCheck();

    template<typename T>
    void formatInteger(T);
};

class Fmt : public Copyable
{
public:
    template<typename T>
    Fmt(const char* fmt, T val);

    const char* data() const
    { return buf_; }

    size_t length() const
    { return len_; }

private:
    char buf_[32];
    size_t len_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}

} // namespace xnet

#endif // LOG_STREAM_H

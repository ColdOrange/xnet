//
// Created by Orange on 7/9/17.
//

#ifndef XNET_BUFFER_H
#define XNET_BUFFER_H

#include <assert.h>
#include <vector>
#include <string>
#include <algorithm>

namespace xnet {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    Buffer()
        : buffer_(kCheapPrepend + kInitialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == kInitialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    Buffer(const Buffer&) = default;
    Buffer& operator=(const Buffer&) = default;

    void swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    const char* readerBegin() const { return begin() + readerIndex_; }

    char* writerBegin() { return begin() + writerIndex_; }
    const char* writerBegin() const { return begin() + writerIndex_; }

    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        readerIndex_ += len;
    }

    void retrieveUntil(const char* ch)
    {
        assert(readerBegin() <= ch && ch <= writerBegin());
        readerIndex_ += (ch - readerBegin());
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(readerBegin(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString()
    {
        std::string result(readerBegin(), readableBytes());
        retrieveAll();
        return result;
    }

    void append(const std::string& str)
    {
        append(str.data(), str.length());
    }

    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, writerBegin());
        writerIndex_ += len;
    }

    void append(const void* data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    void prepend(const void* data, size_t len)
    {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    void shrink(size_t reserve)
    {
        std::vector<char> buf(kCheapPrepend + readableBytes() + reserve);
        std::copy(readerBegin(), readerBegin() + readableBytes(), buf.begin() + kCheapPrepend);
        buf.swap(buffer_);
    }

    ssize_t readFd(int fd, int* savedErrno);

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writerIndex_ + len);
        }
        else {
            // Move readable data to the front, make space inside buffer
            size_t readable = readableBytes();
            std::copy(readerBegin(), readerBegin() + readableBytes(), begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
};

} // namespace xnet

#endif // XNET_BUFFER_H

//
// Created by Orange on 7/9/17.
//

#ifndef XNET_BUFFER_H
#define XNET_BUFFER_H

#include <assert.h>
#include <vector>
#include <string>
#include <algorithm>

#include "SocketOps.h"

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

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    // implicit copy-ctor, move-ctor, dtor and assignment are fine

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
        if (len < readableBytes()) {
            readerIndex_ += len;
        }
        else {
            retrieveAll();
        }
    }

    void retrieveInt64()
    {
        retrieve(sizeof(int64_t));
    }

    void retrieveInt32()
    {
        retrieve(sizeof(int32_t));
    }

    void retrieveInt16()
    {
        retrieve(sizeof(int16_t));
    }

    void retrieveInt8()
    {
        retrieve(sizeof(int8_t));
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

    void appendInt64(int64_t x)
    {
        int64_t be64 = sockops::hostToNetwork64(x);
        append(&be64, sizeof(be64));
    }

    void appendInt32(int32_t x)
    {
        int32_t be32 = sockops::hostToNetwork32(x);
        append(&be32, sizeof(be32));
    }

    void appendInt16(int16_t x)
    {
        int16_t be16 = sockops::hostToNetwork16(x);
        append(&be16, sizeof(be16));
    }

    void appendInt8(int8_t x)
    {
        append(&x, sizeof(x));
    }

    void unwrite(size_t len)
    {
        assert(len <= readableBytes());
        writerIndex_ -= len;
    }

    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    int64_t readInt64()
    {
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }

    int32_t readInt32()
    {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }

    int16_t readInt16()
    {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    int8_t readInt8()
    {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    int64_t peekInt64() const
    {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, readerBegin(), sizeof(be64));
        return sockops::networkToHost64(be64);
    }

    int32_t peekInt32() const
    {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, readerBegin(), sizeof(be32));
        return sockops::networkToHost32(be32);
    }

    int16_t peekInt16() const
    {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, readerBegin(), sizeof(be16));
        return sockops::networkToHost16(be16);
    }

    int8_t peekInt8() const
    {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *readerBegin();
        return x;
    }

    void prepend(const void* data, size_t len)
    {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    void prependInt64(int64_t x)
    {
        int64_t be64 = sockops::hostToNetwork64(x);
        prepend(&be64, sizeof(be64));
    }

    void prependInt32(int32_t x)
    {
        int32_t be32 = sockops::hostToNetwork32(x);
        prepend(&be32, sizeof(be32));
    }

    void prependInt16(int16_t x)
    {
        int16_t be16 = sockops::hostToNetwork16(x);
        prepend(&be16, sizeof(be16));
    }

    void prependInt8(int8_t x)
    {
        prepend(&x, sizeof(x));
    }

    void shrink(size_t reserve)
    {
        std::vector<char> buf(kCheapPrepend + readableBytes() + reserve);
        std::copy(readerBegin(), readerBegin() + readableBytes(), buf.begin() + kCheapPrepend);
        buf.swap(buffer_);
    }

    size_t internalCapacity() const
    {
        return buffer_.capacity();
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

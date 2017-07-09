//
// Created by Orange on 7/9/17.
//

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#include "Buffer.h"

using namespace xnet;

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    char extraBuffer[65535];
    struct iovec vec[2];
    const size_t writable = writableBytes();

    vec[0].iov_base = writerBegin();
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuffer;
    vec[1].iov_len = sizeof(extraBuffer);

    const ssize_t n = readv(fd, vec, 2);
    if (n < 0) {
        *savedErrno = errno;
    }
    else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    }
    else {
        writerIndex_ = buffer_.size();
        append(extraBuffer, n - writable);
    }
    return n;
}

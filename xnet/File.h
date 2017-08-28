//
// Created by Orange on 8/27/17.
//

#ifndef XNET_FILE_H
#define XNET_FILE_H

#include <stdio.h>
#include <string.h>

#include "Logging.h" // for strerror_tl

namespace xnet {

//
// File for appending.
//
class AppendableFile : Noncopyable
{
public:
    explicit AppendableFile(const std::string& filename)
        : fp_(::fopen(filename.data(), "ae")), // e for O_CLOEXEC
          writtenBytes_(0)
    {
        ::setbuffer(fp_, buffer_, sizeof buffer_);
    }

    ~AppendableFile()
    {
        ::fclose(fp_);
    }

    void append(const char* data, size_t len)
    {
        size_t written = 0;
        size_t remain = len;
        while (remain > 0) {
            size_t x = ::fwrite_unlocked(data + written, 1, remain, fp_);
            if (x == 0) {
                int err = ferror(fp_);
                if (err) {
                    fprintf(stderr, "AppendableFile::append() failed %s\n", strerror_tl(err));
                }
                break;
            }
            written += x;
            remain -= x;
        }
        writtenBytes_ += len;
    }

    void flush()
    {
        ::fflush(fp_);
    }

    size_t writtenBytes() const
    { return writtenBytes_; }

private:
    FILE* fp_;
    char buffer_[64 * 1024];
    size_t writtenBytes_;
};

//
// File for randomly reading and writing.
//
class RandomAccessFile : Noncopyable
{
public:
    explicit RandomAccessFile(const std::string& filename)
    {
        if ((fp_ = ::fopen(filename.data(), "r+e")) == nullptr) {
            fp_ = ::fopen(filename.data(), "w+e");
        }
    }

    ~RandomAccessFile()
    {
        ::fclose(fp_);
    }

    const char* buffer() const
    { return buffer_; }

    // Seek to pos from the file starting.
    void seek(long pos)
    {
        ::fseek(fp_, pos, SEEK_SET);
    }

    // Read up to n bytes from the file starting at offset, written to buffer_.
    size_t read(long offset, size_t n)
    {
        seek(offset);
        size_t r = fread_unlocked(buffer_, 1, n, fp_);
        if (r < n) {
            if (!feof(fp_)) {
                int err = ferror(fp_);
                if (err) {
                    fprintf(stderr, "RandomAccessFile::read() failed %s\n", strerror_tl(err));
                }
            }
        }
        return r;
    }

    // Write n bytes to the file starting at offset, old data is truncated.
    void write(long offset, const char* data, size_t n)
    {
        seek(offset); // TODO: verify
        size_t written = 0;
        size_t remain = n;
        while (remain > 0) {
            size_t x = ::fwrite_unlocked(data + written, 1, remain, fp_);
            if (x == 0) {
                int err = ferror(fp_);
                if (err) {
                    fprintf(stderr, "RandomAccessFile::write() failed %s\n", strerror_tl(err));
                }
                break;
            }
            written += x;
            remain -= x;
        }
    }

    void flush()
    {
        ::fflush(fp_);
    }

private:
    FILE* fp_;
    char buffer_[64 * 1024];
};

} // namespace xnet

#endif // XNET_FILE_H

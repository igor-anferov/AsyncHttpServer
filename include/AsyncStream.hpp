#pragma once

#include <stddef.h>

#include <functional>

class AsyncInputStream {
public:
    using Cb = std::function<void(bool success, const char* buf, size_t size)>;

    virtual ~AsyncInputStream() = default;

    virtual void read_till(const char* delimiter, size_t del_size, Cb) = 0;
    virtual void read(size_t count, Cb) = 0;
};

class AsyncOutputStream {
public:
    using Cb = std::function<void(bool success)>;

    virtual ~AsyncOutputStream() = default;

    virtual void write(const char* buf, size_t count, Cb) = 0;
};

class AsyncIOStream:
    virtual public AsyncInputStream,
    virtual public AsyncOutputStream
{
};

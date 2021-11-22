#pragma once

#include "AsyncStream.hpp"
#include "EventLoop.hpp"
#include "File.hpp"

class AsyncFileInputStream: virtual public AsyncInputStream {
    EventLoop& e;
    ReadableFile& file;
    size_t buffer_size;
    char* buffer;
    size_t buffer_begin = 0;
    size_t buffer_end = 0;

    static const size_t readsize = 4096;

    void trim_prefix();
    void expand();
    void notify(size_t notify_size, size_t skip_size, Cb& cb);
    void fill(std::function<void(bool success)>);

public:
    AsyncFileInputStream(EventLoop& e, ReadableFile& file):
        e(e),
        file(file),
        buffer_size(readsize),
        buffer((char*)malloc(buffer_size))
    {}
    ~AsyncFileInputStream() { free(buffer); }

    virtual void read_till(const char* delimiter, size_t del_size, Cb) override;
    virtual void read(size_t count, Cb) override;
};

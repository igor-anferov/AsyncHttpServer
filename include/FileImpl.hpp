#pragma once

#include "EventLoop.hpp"
#include "File.hpp"

class ReadAwaitableFileImpl: virtual public ReadAwaitableFile {
    EventLoop& e;
public:
    ReadAwaitableFileImpl(int fd, EventLoop& e): File(fd), e(e) {}
    void on_readable(Cb cb) const override { e.on_readable(get_fd(), cb); }
};

class WriteAwaitableFileImpl: virtual public WriteAwaitableFile {
    EventLoop& e;
public:
    WriteAwaitableFileImpl(int fd, EventLoop& e): File(fd), e(e) {}
    void on_writable(Cb cb) const override { e.on_writable(get_fd(), cb); }
};

class ReadableFileImpl:
    virtual public ReadableFile,
    public ReadAwaitableFileImpl
{
    EventLoop& e;
public:
    ReadableFileImpl(int fd, EventLoop& e):
        File(fd),
        ReadAwaitableFileImpl(fd, e),
        e(e)
    {}
    void read(char* buf, size_t buf_size, Cb) override;
};

class WritableFileImpl:
    virtual public WritableFile,
    public WriteAwaitableFileImpl
{
    EventLoop& e;
public:
    WritableFileImpl(int fd, EventLoop& e):
        File(fd),
        WriteAwaitableFileImpl(fd, e),
        e(e)
    {}
    void write(const char* buf, size_t buf_size, Cb) override;
};

class ReadWritableFileImpl:
    public ReadWritableFile,
    public ReadableFileImpl,
    public WritableFileImpl
{
public:
    ReadWritableFileImpl(int fd, EventLoop& e):
        File(fd),
        ReadableFileImpl(fd, e),
        WritableFileImpl(fd, e)
    {}
};

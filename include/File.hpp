#pragma once

#include <unistd.h>

#include <functional>

class File {
    int fd;
public:
    File(int fd): fd{ fd } {}
    virtual ~File() { ::close(fd); }
    int get_fd() const { return fd; }
};

class ReadAwaitableFile: virtual public File {
public:
    using Cb = std::function<void(bool success)>;
    virtual void on_readable(Cb) const = 0;
};

class WriteAwaitableFile: virtual public File {
public:
    using Cb = std::function<void(bool success)>;
    virtual void on_writable(Cb) const = 0;
};

class ReadableFile: virtual public ReadAwaitableFile {
public:
    using Cb = std::function<void(ssize_t read)>;
    virtual void read(char* buf, size_t buf_size, Cb) = 0;
};

class WritableFile: virtual public WriteAwaitableFile {
public:
    using Cb = std::function<void(ssize_t written)>;
    virtual void write(const char* buf, size_t buf_size, Cb) = 0;
};

class ReadWritableFile: virtual public ReadableFile, virtual public WritableFile {
};

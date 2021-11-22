#include "FileImpl.hpp"

#include <unistd.h>

void ReadableFileImpl::read(char* buf, size_t buf_size, Cb cb)
{
    on_readable([this, buf, buf_size, cb] (bool success) {
        cb(success ? ::read(get_fd(), buf, buf_size) : -1);
    });
}

void WritableFileImpl::write(const char* buf, size_t buf_size, Cb cb)
{
    on_writable([this, buf, buf_size, cb] (bool success) {
        cb(success ? ::write(get_fd(), buf, buf_size) : -1);
    });
}

#include "AsyncFileInputStream.hpp"

#include "string.h"

void AsyncFileInputStream::notify(size_t notify_size, size_t skip_size, Cb& cb)
{
    e.schedule([this, cb, buf = buffer + buffer_begin, size = notify_size] {
        cb(true, buf, size);
    });
    buffer_begin += notify_size + skip_size;
    if (buffer_begin == buffer_end)
        buffer_begin = buffer_end = 0;
}

void AsyncFileInputStream::trim_prefix()
{
    for (size_t to = 0, from = buffer_begin; from < buffer_end; ++to, ++from)
        buffer[to] = buffer[from];

    buffer_end -= buffer_begin;
    buffer_begin = 0;
}

void AsyncFileInputStream::expand()
{
    buffer_size = buffer_end + readsize;
    buffer = (char*)realloc(buffer, buffer_size);
}

void AsyncFileInputStream::fill(std::function<void(bool success)> cb)
{
    if (buffer_begin > readsize)
        trim_prefix();
    if (buffer_size - buffer_end < readsize)
        expand();
    file.read(buffer + buffer_end, buffer_size - buffer_end,
        [this, cb] (ssize_t read) {
            if (read > 0)
                buffer_end += read;
            cb(read > 0);
        }
    );
}

void AsyncFileInputStream::read_till(const char *delimiter, size_t del_size, Cb cb)
{
    if (char* pos = (char*)memmem(buffer + buffer_begin, buffer_end - buffer_begin, delimiter, del_size))
    {
        notify(pos - (buffer + buffer_begin), del_size, cb);
    }
    else
    {
        fill([this, delimiter, del_size, cb] (bool success) {
            if (!success)
                return cb(false, NULL, 0);
            read_till(delimiter, del_size, cb);
        });
    }
}

void AsyncFileInputStream::read(size_t count, Cb cb)
{
    if (buffer_end - buffer_begin >= count) {
        notify(count, 0, cb);
    } else {
        fill([this, count, cb] (bool success) {
            if (!success)
                return cb(false, NULL, 0);
            read(count, cb);
        });
    }
}

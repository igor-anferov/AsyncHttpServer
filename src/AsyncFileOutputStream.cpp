#include "AsyncFileOutputStream.hpp"

void AsyncFileOutputStream::write(const char *buf, size_t count, Cb cb)
{
    if (count == 0)
        return e.schedule([cb] { cb(true); });

    file.write(buf, count, [this, buf, count, cb] (ssize_t written) {
        if (written <= 0)
            return cb(false);

        if (count == written)
            return cb(true);

        write(buf + written, count - written, cb);
    });
}

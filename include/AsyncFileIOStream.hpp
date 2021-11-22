#pragma once

#include "AsyncStream.hpp"
#include "AsyncFileInputStream.hpp"
#include "AsyncFileOutputStream.hpp"

class AsyncFileIOStream:
    public AsyncIOStream,
    public AsyncFileInputStream,
    public AsyncFileOutputStream
{
    ReadWritableFile* file;

public:
    AsyncFileIOStream(EventLoop& e, ReadWritableFile* file):
        AsyncFileInputStream(e, *file),
        AsyncFileOutputStream(e, *file),
        file(file)
    {}
    ~AsyncFileIOStream() { delete file; }
};

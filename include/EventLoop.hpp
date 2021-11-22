#pragma once

#include <functional>

class EventLoop {
public:
    using ResCb = std::function<void(bool success)>;
    using Cb = std::function<void()>;

    virtual ~EventLoop() = default;

    virtual void on_readable(int fd, ResCb) = 0;
    virtual void on_writable(int fd, ResCb) = 0;
    virtual void schedule(Cb) = 0;
};

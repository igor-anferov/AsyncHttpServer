#pragma once

#include "EventLoop.hpp"

#include <sys/select.h>

#include <queue>
#include <unordered_map>

class SelectBasedEventLoop: public EventLoop {
    std::unordered_map<int, ResCb> on_readable_callbacks;
    std::unordered_map<int, ResCb> on_writable_callbacks;
    std::queue<Cb> scheduled_callbacks;

    void schedule(std::unordered_map<int, ResCb>&, const fd_set& ok_fds, const fd_set& err_fds, int& unscheduled);

public:
    void on_readable(int fd, ResCb) override;
    void on_writable(int fd, ResCb) override;
    void schedule(Cb) override;

    void run_untill_complete();
};

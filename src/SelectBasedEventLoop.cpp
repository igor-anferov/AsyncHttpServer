#include "SelectBasedEventLoop.hpp"

#include "Utils.hpp"

void SelectBasedEventLoop::on_readable(int fd, ResCb cb)
{
    const auto& [it, inserted] = on_readable_callbacks.emplace(fd, cb);
    ASSERT(inserted, "trying to track same fd twice");
}

void SelectBasedEventLoop::on_writable(int fd, ResCb cb)
{
    const auto& [it, inserted] = on_writable_callbacks.emplace(fd, cb);
    ASSERT(inserted, "trying to track same fd twice");
}

void SelectBasedEventLoop::schedule(Cb cb)
{
    scheduled_callbacks.push(cb);
}

void SelectBasedEventLoop::schedule(std::unordered_map<int, ResCb>& cbs, const fd_set& ok_fds, const fd_set& err_fds, int& unscheduled)
{
    for (auto it = cbs.begin(); it != cbs.end();) {
        if (!unscheduled)
            break;

        auto& [fd, cb] = *it;

        if (FD_ISSET(fd, &ok_fds) || FD_ISSET(fd, &err_fds)) {
            --unscheduled;
            schedule([cb = cb, success = (bool)FD_ISSET(fd, &ok_fds)] {
                cb(success);
            });
            it = cbs.erase(it);
        } else {
            ++it;
        }
    }
}

void SelectBasedEventLoop::run_untill_complete()
{
    while (!on_readable_callbacks.empty() || !on_writable_callbacks.empty()) {
        fd_set track_readability;
        FD_ZERO(&track_readability);

        fd_set track_writability;
        FD_ZERO(&track_writability);

        fd_set track_errors;
        FD_ZERO(&track_errors);

        int max_fd = -1;

        for (auto& [fd, cb]: on_readable_callbacks) {
            if (fd > max_fd)
                max_fd = fd;
            FD_SET(fd, &track_readability);
            FD_SET(fd, &track_errors);
        }

        for (auto& [fd, cb]: on_writable_callbacks) {
            if (fd > max_fd)
                max_fd = fd;
            FD_SET(fd, &track_writability);
            FD_SET(fd, &track_errors);
        }

        int rc;
        while ((rc = select(max_fd + 1, &track_readability, &track_writability, &track_errors, NULL)) == -1) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            FATAL("unexpected 'select' error");
        }

        schedule(on_readable_callbacks, track_readability, track_errors, rc);
        schedule(on_writable_callbacks, track_writability, track_errors, rc);

        ASSERT(!rc, "couldn't find callbacks for some fds");

        while (!scheduled_callbacks.empty()) {
            scheduled_callbacks.front()();
            scheduled_callbacks.pop();
        }
    }
}

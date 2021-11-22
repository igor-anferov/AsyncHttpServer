#pragma once

#include "FileImpl.hpp"
#include "ListeningSocket.hpp"

class ListeningNetworkSocket:
    public ReadAwaitableFileImpl,
    public ListeningSocket
{
    EventLoop& e;

    static int make_listening_socket(const char* ip, uint16_t port);

public:
    ListeningNetworkSocket(const char* ip, uint16_t port, EventLoop& e):
        File(make_listening_socket(ip, port)),
        ReadAwaitableFileImpl(get_fd(), e),
        e(e)
    {}
    void start_listening(ClientAcceptor&) override;
};

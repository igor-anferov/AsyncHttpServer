#include "ListeningNetworkSocket.hpp"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "AsyncFileIOStream.hpp"
#include "Utils.hpp"

int ListeningNetworkSocket::make_listening_socket(const char* ip, uint16_t port)
{
    struct sockaddr_in addr{ .sin_family = AF_INET };
    int rc = inet_aton(ip, &addr.sin_addr);
    ASSERT(rc == 1, "failed to parse IP address");
    addr.sin_port = htons(port);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(fd >= 0, "failed to create listening socket");

    int opt_value = 1;
    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
    ASSERT(!rc, "failed to set SO_REUSEADDR option for listening socket");

    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    rc = fcntl(fd, F_SETFL, flags);
    ASSERT(rc != -1, "failed to set O_NONBLOCK flag on listening socket");

    rc = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    ASSERT(!rc, "listening socket bind failed");

    rc = listen(fd, 4096);
    ASSERT(!rc, "listen() failed");

    return fd;
}

void ListeningNetworkSocket::start_listening(ClientAcceptor& conn_acceptor)
{
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int fd = accept(get_fd(), (struct sockaddr*)&client_addr, &len);
        if (fd == -1 && errno == EWOULDBLOCK)
            break;
        ASSERT(fd >= 0, "failed to 'accept()' client");
        fprintf(stderr, "new client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        int rc = fcntl(fd, F_SETFL, flags);
        ASSERT(rc != -1, "failed to set O_NONBLOCK flag on client socket");

        conn_acceptor.accept(new AsyncFileIOStream(e, new ReadWritableFileImpl(fd, e)));
    }
    on_readable([this, &conn_acceptor] (bool success) {
        if (success)
            start_listening(conn_acceptor);
        else
            fprintf(stderr, "failed to await readability of listening socket");
    });
}

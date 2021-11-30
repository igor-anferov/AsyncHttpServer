#include <signal.h>

#include <iostream>
#include <string>

#include <magic.h>

#include "SelectBasedEventLoop.hpp"
#include "ListeningNetworkSocket.hpp"
#include "EchoClientAcceptor.hpp"
#include "HttpClientAcceptor.hpp"
#include "Utils.hpp"

int main(int argc, const char * argv[]) {
    signal(SIGPIPE, SIG_IGN);

    errno = 0;
    ASSERT(argc == 3, "2 arguments expected: ip and port");
    std::cout << "Starting server..." << std::endl;
    SelectBasedEventLoop loop;
    ListeningNetworkSocket listening_socket(argv[1], std::stoi(argv[2]), loop);
//    EchoClientAcceptor client_acceptor;

    magic_t magic = magic_open(MAGIC_MIME);
    int rc = magic_load(magic, NULL);
    ASSERT(!rc, "failed to load 'magic' database");
    HttpClientAcceptor client_acceptor(magic);

    listening_socket.start_listening(client_acceptor);

    loop.run_untill_complete();

    magic_close(magic);

    return 0;
}

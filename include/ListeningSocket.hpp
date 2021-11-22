#pragma once

#include "File.hpp"
#include "ClientAcceptor.hpp"

class ListeningSocket: virtual public ReadAwaitableFile {
public:
    virtual ~ListeningSocket() = default;
    virtual void start_listening(ClientAcceptor&) = 0;
};

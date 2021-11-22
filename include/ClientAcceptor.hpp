#pragma once

#include "AsyncStream.hpp"

class ClientAcceptor {
public:
    virtual ~ClientAcceptor() = default;
    virtual void accept(AsyncIOStream*) = 0;
};

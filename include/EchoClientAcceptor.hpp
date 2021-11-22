#pragma once

#include "ClientAcceptor.hpp"

class EchoClientAcceptor: public ClientAcceptor {
    class EchoClientProcessor {
        AsyncIOStream* stream;
    public:
        EchoClientProcessor(AsyncIOStream* stream): stream(stream) {}
        ~EchoClientProcessor() { delete stream; }

        void process(std::function<void()> end_cb);
    };


public:
    void accept(AsyncIOStream*) override;
};

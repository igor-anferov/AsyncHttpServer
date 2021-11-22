#include "EchoClientAcceptor.hpp"

void EchoClientAcceptor::EchoClientProcessor::process(std::function<void()> end_cb)
{
    stream->read_till(
        "\n", 1,
        [this, end_cb] (bool success, const char* buf, size_t size) {
            if (!success)
                return end_cb();
            stream->write(buf, size + 1, [this, end_cb] (bool success) {
                if (!success)
                    return end_cb();
                process(end_cb);
            });
        }
    );
}

void EchoClientAcceptor::accept(AsyncIOStream* stream)
{
    EchoClientProcessor* processor = new EchoClientProcessor(stream);
    processor->process([processor] {
        delete processor;
    });
}

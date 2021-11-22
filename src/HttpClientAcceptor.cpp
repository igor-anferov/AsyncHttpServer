#include "HttpClientAcceptor.hpp"

#include <strings.h>

#include <iostream>
#include <string_view>

#include "Utils.hpp"

void HttpClientAcceptor::HttpClientProcessor::reply(int code, const char* reason)
{
    int size = snprintf(write_buffer, sizeof(write_buffer), "%s %d %s\r\nConnection: %s\r\n\r\n", proto.c_str(), code, reason, keep_alive ? "keep-alive" : "close");

    ASSERT(size > 0 && size < sizeof(write_buffer), "too small reply buffer");

    stream->write(write_buffer, size, [this] (bool success) {
        if (!success || !keep_alive)
            end_cb();
        else
            get_start_line();
    });
}

void HttpClientAcceptor::HttpClientProcessor::reply()
{
    last_resource.seekg(0, std::ios_base::end);

    size_t total_size = last_resource.tellg();

    last_resource.seekg(0, std::ios_base::beg);

    char beginning[2048];

    ssize_t size = last_resource.read(beginning, sizeof(beginning)).gcount();

    const char* mime = magic_buffer(magic, beginning, size);

    int writebuf_filled = snprintf(
        write_buffer, sizeof(write_buffer),
        "%s 200 Ok\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: %s\r\n"
        "\r\n",
        proto.c_str(),
        mime ?: "application/octet-stream",
        total_size,
        keep_alive ? "keep-alive" : "close"
    );

    ASSERT(writebuf_filled > 0 && writebuf_filled + size < sizeof(write_buffer), "too small reply buffer");

    memcpy(write_buffer + writebuf_filled, beginning, size);
    writebuf_filled += size;

    stream->write(write_buffer, writebuf_filled, [this] (bool success) {
        if (!success)
            return end_cb();
        if (last_resource)
            return next_file_part();
        if (keep_alive)
            return get_start_line();
        end_cb();
    });
}

void HttpClientAcceptor::HttpClientProcessor::next_file_part()
{
    ssize_t size = last_resource.read(write_buffer, sizeof(write_buffer)).gcount();
    stream->write(write_buffer, size, [this] (bool success) {
        if (!success)
            return end_cb();
        if (last_resource)
            return next_file_part();
        if (keep_alive)
            return get_start_line();
        end_cb();
    });
}

void HttpClientAcceptor::HttpClientProcessor::get_start_line()
{
    keep_alive = false;
    stream->read_till(
        "\r\n", 2,
        [this] (bool success, const char* buf, size_t size) {
            if (!success)
                return end_cb();

            std::string_view start_string(buf, size);

            if (start_string.substr(0, 5) != "GET /")
                return reply(405, "Method not allowed");

            start_string.remove_prefix(5);

            ssize_t resource_end = start_string.find(" ");

            if (resource_end == start_string.npos)
                return reply(400, "Bad request");

            proto = start_string.substr(resource_end + 1);

            start_string = start_string.substr(0, resource_end);

            last_resource = std::ifstream(std::string(start_string));

            get_header();
        }
    );
}

void HttpClientAcceptor::HttpClientProcessor::get_header()
{
    stream->read_till(
        "\r\n", 2,
        [this] (bool success, const char* buf, size_t size) {
            if (!success)
                return end_cb();

            if (!size)
                return request_finished();

            std::string_view header(buf, size);

            ssize_t key_end = header.find(": ");

            if (key_end == header.npos)
                return reply(400, "Bad request");

            if (!strncasecmp("Connection", header.data(), key_end)) {
                header.remove_prefix(key_end + 2);
                if (!strncasecmp("keep-alive", header.data(), header.size()))
                    keep_alive = true;
            }

            get_header();
        }
    );
}

void HttpClientAcceptor::HttpClientProcessor::request_finished()
{
    if (!last_resource)
        return reply(404, "Not found");
    else
        reply();
}

void HttpClientAcceptor::HttpClientProcessor::process(EndCb end_cb)
{
    this->end_cb = end_cb;
    get_start_line();
}

void HttpClientAcceptor::accept(AsyncIOStream* stream)
{
    HttpClientProcessor* processor = new HttpClientProcessor(magic, stream);
    processor->process([processor] {
        delete processor;
    });
}

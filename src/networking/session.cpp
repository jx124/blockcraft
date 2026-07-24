#include "networking/session.hpp"
#include "utils/logger.hpp"

void Session::start(PacketHandler on_packet, CloseHandler on_close) {
    this->on_packet = std::move(on_packet);
    this->on_close = std::move(on_close);
    asio::co_spawn(socket.get_executor(), read_loop(), asio::detached);
}

void Session::send(Packet packet) {
    write_queue.push_back(std::move(packet));
    if (!is_writing) {
        is_writing = true;
        asio::co_spawn(socket.get_executor(), write_loop(), asio::detached);
    }
}

void Session::close() {
    if (is_closing) {
        return;
    }
    is_closing = true;

    asio::error_code ec;
    auto _ = socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

    socket.close();
    if (on_close) {
        on_close(shared_from_this());
    }
}

asio::ip::tcp::endpoint Session::remote_endpoint() const {
    asio::error_code ec;
    auto endpoint = socket.remote_endpoint(ec);
    if (ec) {
        log_error("Error resolving endpoints: %s", ec.message().c_str());
        return {};
    }
    return endpoint;
}

asio::awaitable<void> Session::read_loop() {
    auto self = shared_from_this();

    while (true) {
        PacketHeader header{};
        asio::error_code ec;
        unsigned long length;
        std::tie(ec, length) = co_await asio::async_read(socket,
                                                         asio::buffer(&header, sizeof(PacketHeader)),
                                                         asio::as_tuple(asio::use_awaitable));
        if (ec) {
            if (ec != asio::error::eof) {
                log_error("Error reading header: %s", ec.message().c_str());
            }
            break;
        }

        std::vector<std::byte> payload((int)header.length);
        if (header.length > 0) {
            std::tie(ec, length) = co_await asio::async_read(socket,
                                                             asio::buffer(payload),
                                                             asio::as_tuple(asio::use_awaitable));
            if (ec) {
                if (ec != asio::error::eof) {
                    log_error("Error reading payload: %s", ec.message().c_str());
                }
                break;
            }
        }

        if (on_packet) {
            on_packet(self, Packet{ header, std::move(payload) });
        }
    }
    close();
}

asio::awaitable<void> Session::write_loop() {
    asio::error_code ec;
    unsigned long length;

    while (!write_queue.empty()) {
        auto data = write_queue.front().serialize();
        write_queue.pop_front();
        std::tie(ec, length) = co_await asio::async_write(socket,
                                                          asio::buffer(data),
                                                          asio::as_tuple(asio::use_awaitable));

        if (ec) {
            if (ec != asio::error::eof) {
                log_error("Error writing packet: %s", ec.message().c_str());
            }
            close();
            break;
        }
    }
    is_writing = false;
}

#pragma once

#include "asio.hpp"
#include "networking/packet.hpp"

#include <functional>
#include <memory>
#include <deque>

class Session : public std::enable_shared_from_this<Session> {
public:
    using PacketHandler = std::function<void(std::shared_ptr<Session>, Packet)>;
    using CloseHandler = std::function<void(std::shared_ptr<Session>)>;

    explicit Session(asio::ip::tcp::socket socket) : socket(std::move(socket)) {};

    void start(PacketHandler on_packet, CloseHandler on_close);
    void send(Packet packet);
    void close();
    asio::ip::tcp::endpoint remote_endpoint() const;

private:
    asio::awaitable<void> read_loop();
    asio::awaitable<void> write_loop();

    asio::ip::tcp::socket socket;
    PacketHandler on_packet{};
    CloseHandler on_close{};
    std::deque<Packet> write_queue{};
    bool is_writing = false;
    bool is_closing = false;
};


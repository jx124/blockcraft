#pragma once

#include "asio.hpp"
#include "networking/session.hpp"
#include "networking/packet.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

class ServerInterface {
public:
    using PacketHandler = std::function<void(std::shared_ptr<Session>, Packet)>;

    explicit ServerInterface(uint16_t port);

    void register_packet_handler(PacketType type, PacketHandler handler);
    void send_to(std::shared_ptr<Session> session, Packet packet);
    void broadcast(const Packet& packet, std::shared_ptr<Session> exclude = nullptr);

    void poll();
    size_t client_count() const;

private:
    asio::awaitable<void> accept_loop();

    asio::io_context io_context{};
    asio::ip::tcp::acceptor acceptor;
    std::unordered_map<PacketType, PacketHandler> handlers{};
    std::unordered_map<uint64_t, std::shared_ptr<Session>> sessions{};
    uint64_t next_id = 1;
};


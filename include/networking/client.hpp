#pragma once

#include "asio.hpp"
#include "networking/session.hpp"
#include "networking/packet.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class ClientInterface {
public:
    using PacketHandler = std::function<void(Packet)>;
    using ConnectHandler = std::function<void()>;
    using DisconnectHandler = std::function<void()>;

    void register_packet_handler(PacketType type, PacketHandler handler);
    void register_connect_handler(ConnectHandler handler);
    void register_disconnect_handler(DisconnectHandler handler);

    void connect(const std::string& host, uint16_t port);
    void send(Packet packet);
    void disconnect();

    bool is_connected() const;
    void poll();

private:
    asio::awaitable<void> connect_coroutine(std::string host, std::string port);

    asio::io_context io_context{};
    std::shared_ptr<Session> session{};
    std::unordered_map<PacketType, PacketHandler> handlers{};
    ConnectHandler on_connect{};
    DisconnectHandler on_disconnect{};
};


#include "networking/server.hpp"
#include "utils/logger.hpp"

ServerInterface::ServerInterface(uint16_t port)
        : acceptor(io_context, { asio::ip::tcp::v4(), port }) {
    asio::co_spawn(io_context, accept_loop(), asio::detached);
}

void ServerInterface::register_packet_handler(PacketType type, PacketHandler handler) {
    handlers[type] = std::move(handler);
}

void ServerInterface::send_to(std::shared_ptr<Session> session, Packet packet) {
    session->send(std::move(packet));
}

void ServerInterface::broadcast(const Packet& packet, std::shared_ptr<Session> exclude) {
    for (auto& [id, session] : sessions) {
        if (session != exclude) {
            session->send(packet);
        }
    }
}

void ServerInterface::poll() {
    io_context.restart();
    io_context.poll();
}

size_t ServerInterface::client_count() const {
    return sessions.size();
}

asio::awaitable<void> ServerInterface::accept_loop() {
    while (true) {
        asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);

        uint64_t id = next_id++;
        auto session = std::make_shared<Session>(std::move(socket));

        // copy endpoint first so we still have it when disconnecting
        auto endpoint = session->remote_endpoint();
        log_debug("[Server] Client %ld connected (%s)", id, endpoint);

        session->start(
            [this](std::shared_ptr<Session> session, Packet packet){
                if (handlers.contains(packet.header.type)) {
                    handlers[packet.header.type](session, packet);
                }
            },
            [this, id, endpoint](std::shared_ptr<Session>) {
                log_debug("[Server] Client %ld disconnected (%s)", id, endpoint);
                sessions.erase(id);
            }
        );
    }
}

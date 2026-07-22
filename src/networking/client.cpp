#include "networking/client.hpp"
#include "utils/logger.hpp"

void ClientInterface::register_packet_handler(PacketType type, PacketHandler handler) {
    handlers[type] = std::move(handler);
}

void ClientInterface::register_connect_handler(ConnectHandler handler) {
    on_connect = std::move(handler);
}

void ClientInterface::register_disconnect_handler(DisconnectHandler handler) {
    on_disconnect = std::move(handler);
}

void ClientInterface::connect(const std::string& host, uint16_t port) {
    asio::co_spawn(io_context, connect_coroutine(host, std::to_string(port)), asio::detached);
}

void ClientInterface::send(Packet packet) {
    if (session) {
        session->send(std::move(packet));
    }
}

void ClientInterface::disconnect() {
    if (session) {
        session->close();
        session.reset();
    }
}

bool ClientInterface::is_connected() const {
    return session != nullptr;
}

void ClientInterface::poll() {
    io_context.restart();
    io_context.poll();
}

asio::awaitable<void> ClientInterface::connect_coroutine(std::string host, std::string port) {
    auto executor = co_await asio::this_coro::executor;

    asio::ip::tcp::resolver resolver(executor);
    asio::error_code ec;
    asio::ip::basic_resolver_results<asio::ip::tcp> endpoints;

    std::tie(ec, endpoints) = co_await resolver.async_resolve(host,
                                                              port,
                                                              asio::as_tuple(asio::use_awaitable));

    if (ec) {
        log_error("Error resolving endpoint: %s", ec.message());
        co_return;
    }

    asio::ip::tcp::socket socket(executor);
    asio::ip::basic_endpoint<asio::ip::tcp> connection_endpoint;

    std::tie(ec, connection_endpoint) = co_await asio::async_connect(socket,
                                                                     endpoints,
                                                                     asio::as_tuple(asio::use_awaitable));

    if (ec) {
        log_error("Error resolving endpoint (%s): %s", connection_endpoint, ec.message());
        co_return;
    }

    session = std::make_shared<Session>(std::move(socket));
    session->start(
        [this](std::shared_ptr<Session>, Packet packet){
            if (handlers.contains(packet.header.type)) {
                handlers[packet.header.type](packet);
            }
        },
        [this, host, port](std::shared_ptr<Session> session){
            log_debug("[Client] Disconnected from server (%s:%s)", host, port);
            session.reset();
            if (on_disconnect) {
                on_disconnect();
            }
        }
    );

    log_debug("[Client] Connected to server (%s:%s)", host, port);
    if (on_connect) {
        on_connect();
    }
}


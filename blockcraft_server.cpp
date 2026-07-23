#include "networking/server.hpp"

#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    if (argc != 1 && argc != 2) {
        std::cout << "Usage: blockcraft_server <port>\nHosts server on port 50000 by default if no arguments are supplied." << std::endl;
        return 0;
    }

    int temp_port{};
    if (argc == 1) {
        temp_port = 50000;
    } else {
        temp_port = std::stoi(argv[1]);
    }

    if (temp_port < 0 || temp_port > 65535) {
        std::cout << "Invalid port number" << std::endl;
        return 0;
    }
    uint16_t port = static_cast<uint16_t>(temp_port);

    ServerInterface server(port);

    server.register_packet_handler(PacketType::Ping,
        [&server](std::shared_ptr<Session> session, Packet){
            std::cout << "Ping from " << session->remote_endpoint() << std::endl;
            server.send_to(session, Packet::make(PacketType::Pong, {}));
    });

    using namespace std::chrono_literals;
    while (true) {
        server.poll();
        std::this_thread::sleep_for(100ms);
    }
}

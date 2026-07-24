#include "blocks/chunk.hpp"
#include "networking/server.hpp"

#include <iostream>

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

    server.register_packet_handler(PacketType::ChunkRequest,
        [&server](std::shared_ptr<Session> session, Packet packet){
            ChunkRequest chunk_request = ChunkRequest::deserialize(std::move(packet));

            Chunk chunk(chunk_request.chunk_coords, chunk_request.seed);
            chunk.generate_blocks_from_seed();

            ChunkData chunk_data{
                chunk_request.chunk_coords,
                chunk_request.seed,
                chunk.get_blocks(),
            };

            server.send_to(session, chunk_data.serialize());
    });

    using namespace std::chrono_literals;
    while (true) {
        server.poll();
    }
}

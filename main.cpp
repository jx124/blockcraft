#include "app/client.hpp"

#include <cstdint>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage:\n\tHosting a server/singleplayer: --host <port>\n\tJoining a server: --join <hostname> <port>" << std::endl;
        return 0;
    }

    std::string mode_str(argv[1]);
    if (mode_str != "--host" && mode_str != "--join") {
        std::cout << "Usage:\n\tHosting a server/singleplayer: --host <port>\n\tJoining a server: --join <hostname> <port>" << std::endl;
        return 0;
    }

    ClientApplication::Mode mode{};
    std::string hostname{};
    int temp_port{};
    if (mode_str == "--host") {
        mode = ClientApplication::Mode::Server;
        hostname = "localhost";
        temp_port = std::stoi(argv[2]);
    } else {
        mode = ClientApplication::Mode::Client;
        hostname = argv[2];
        temp_port = std::stoi(argv[3]);
    }

    if (temp_port < 0 || temp_port > 65535) {
        std::cout << "Invalid port number" << std::endl;
        return 0;
    }
    uint16_t port = static_cast<uint16_t>(temp_port);


    ClientApplication application(1920, 1200, mode, hostname, port);
    application.run();
}

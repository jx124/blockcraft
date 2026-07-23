#include "app/client.hpp"

#include <cstdint>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 1 && argc != 3) {
        std::cout << "Usage: blockcraft <hostname> <port>\nJoins localhost:50000 by default if no arguments are supplied." << std::endl;
        return 0;
    }

    std::string hostname{};
    int temp_port{};
    if (argc == 1) {
        hostname = "localhost";
        temp_port = 50000;
    } else {
        hostname = argv[1];
        temp_port = std::stoi(argv[2]);
    }

    if (temp_port < 0 || temp_port > 65535) {
        std::cout << "Invalid port number" << std::endl;
        return 0;
    }
    uint16_t port = static_cast<uint16_t>(temp_port);

    ClientApplication application(1920, 1200, hostname, port);
    application.run();
}

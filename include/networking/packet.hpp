#pragma once

#include <cstdint>
#include <vector>

enum class PacketType : uint16_t {
    Ping = 1,
    Pong,
    PlayerMove,
    ChatMsg,
};

struct PacketHeader {
    PacketType type{}; // 2 bytes: packet type
    uint16_t length{}; // 2 bytes: length of payload (excluding header)
};

struct Packet {
    PacketHeader header{};
    std::vector<uint8_t> payload{};

    // Convert packet into a flat buffer
    std::vector<uint8_t> serialize() const;
    static Packet make(PacketType type, std::vector<uint8_t> payload);
};


#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

enum class PacketType : uint16_t {
    Ping = 1,
    Pong,
    ChunkRequest,
    ChunkData,
};

struct PacketHeader {
    PacketType type{}; // 2 bytes: packet type
    uint32_t length{}; // 4 bytes: length of payload (excluding header)
};

struct Packet {
    PacketHeader header{};
    std::vector<std::byte> payload{};

    // Convert packet into a flat buffer
    std::vector<std::byte> serialize() const;
    static Packet make(PacketType type, std::vector<std::byte> payload);
};


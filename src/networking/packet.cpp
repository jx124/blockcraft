#include "networking/packet.hpp"

#include "utils/assert.hpp"
#include <cstddef>
#include <cstring>
#include <limits>

std::vector<std::byte> Packet::serialize() const {
    std::vector<std::byte> buf(sizeof(PacketHeader) + payload.size());
    std::memcpy(buf.data(), &header, sizeof(PacketHeader));
    std::memcpy(buf.data() + sizeof(PacketHeader), payload.data(), payload.size());

    return buf;
}

Packet Packet::make(PacketType type, std::vector<std::byte> payload) {
    Packet packet;
    debug_assert(payload.size() < std::numeric_limits<decltype(PacketHeader::length)>::max(), "Packet too large");
    packet.header = { type, static_cast<decltype(PacketHeader::length)>(payload.size()) };
    packet.payload = std::move(payload);

    return packet;
}

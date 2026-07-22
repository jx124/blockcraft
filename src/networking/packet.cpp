#include "networking/packet.hpp"

#include "utils/assert.hpp"
#include <cstring>
#include <limits>

std::vector<uint8_t> Packet::serialize() const {
    std::vector<uint8_t> buf(sizeof(PacketHeader) + payload.size());
    std::memcpy(buf.data(), &header, sizeof(PacketHeader));
    std::memcpy(buf.data() + sizeof(PacketHeader), payload.data(), payload.size());

    return buf;
}

Packet Packet::make(PacketType type, std::vector<uint8_t> payload) {
    Packet packet;
    debug_assert(payload.size() < std::numeric_limits<decltype(PacketHeader::length)>::max(), "Packet too large");
    packet.header = { type, static_cast<uint16_t>(payload.size()) };
    packet.payload = std::move(payload);

    return packet;
}

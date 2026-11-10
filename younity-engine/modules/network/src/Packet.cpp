/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/Packet
** File description:
** Packet source file
*/

#include "younity/network/Packet.hpp"

#include "younity/network/Limits.hpp"
#include "younity/network/Reader.hpp"

namespace younity::network {

bool encodePacket(const Packet &packet, Writer &out) noexcept
{
    if (!packetTypeFromU8(static_cast<std::uint8_t>(packet.type)).has_value()) {
        return false;
    }
    if ((packet.type == PacketType::Ping || packet.type == PacketType::Pong) &&
        packet.payload.size() != PING_COOKIE_BYTES) {
        return false;
    }
    if ((packet.type == PacketType::Connect || packet.type == PacketType::ConnectAck ||
         packet.type == PacketType::Heartbeat) &&
        !packet.payload.empty()) {
        return false;
    }
    if (out.getSize() + PACKET_HEADER_BYTES + packet.payload.size() > MAX_PACKET_BYTES) {
        return false;
    }
    if (!out.putU16Le(PROTOCOL_MAGIC)) {
        return false;
    }
    if (!out.putU8(PROTOCOL_VERSION)) {
        return false;
    }
    if (!out.putU8(static_cast<std::uint8_t>(packet.type))) {
        return false;
    }
    if (!out.putU32Le(packet.seq)) {
        return false;
    }
    if (!packet.payload.empty() && !out.putBytes(packet.payload)) {
        return false;
    }
    return true;
}

std::expected<Packet, PacketError> decodePacket(std::span<const std::byte> datagram)
{
    if (datagram.size() > MAX_PACKET_BYTES) {
        return std::unexpected(PacketError::TooLarge);
    }
    if (datagram.size() < PACKET_HEADER_BYTES) {
        return std::unexpected(PacketError::Truncated);
    }
    Reader in(datagram);
    const std::expected<std::uint16_t, CodecError> magic = in.getU16Le();

    if (!magic.has_value()) {
        return std::unexpected(PacketError::Truncated);
    }
    if (*magic != PROTOCOL_MAGIC) {
        return std::unexpected(PacketError::BadMagic);
    }
    const std::expected<std::uint8_t, CodecError> version = in.getU8();

    if (!version.has_value()) {
        return std::unexpected(PacketError::Truncated);
    }
    if (*version != PROTOCOL_VERSION) {
        return std::unexpected(PacketError::BadVersion);
    }
    const std::expected<std::uint8_t, CodecError> rawType = in.getU8();

    if (!rawType.has_value()) {
        return std::unexpected(PacketError::Truncated);
    }
    const std::expected<PacketType, PacketError> type = packetTypeFromU8(*rawType);

    if (!type.has_value()) {
        return std::unexpected(type.error());
    }
    const std::expected<std::uint32_t, CodecError> seq = in.getU32Le();

    if (!seq.has_value()) {
        return std::unexpected(PacketError::Truncated);
    }
    std::expected<std::vector<std::byte>, CodecError> payload = in.getBytes(in.getRemaining());

    if (!payload.has_value()) {
        return std::unexpected(PacketError::Truncated);
    }
    if ((*type == PacketType::Ping || *type == PacketType::Pong) && payload->size() != PING_COOKIE_BYTES) {
        return std::unexpected(PacketError::InvalidPayload);
    }
    if ((*type == PacketType::Connect || *type == PacketType::ConnectAck || *type == PacketType::Heartbeat) &&
        !payload->empty()) {
        return std::unexpected(PacketError::InvalidPayload);
    }
    return Packet{.type = *type, .seq = *seq, .payload = std::move(*payload)};
}

} // namespace younity::network

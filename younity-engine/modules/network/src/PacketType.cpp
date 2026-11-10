/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/PacketType
** File description:
** PacketType source file
*/

#include "younity/network/PacketType.hpp"

namespace younity::network {

std::string_view toString(PacketType type) noexcept
{
    switch (type) {
    case PacketType::GameData:
        return "game data";
    case PacketType::Ping:
        return "ping";
    case PacketType::Pong:
        return "pong";
    case PacketType::Connect:
        return "connect";
    case PacketType::ConnectAck:
        return "connect ack";
    case PacketType::Heartbeat:
        return "heartbeat";
    }
    return "unknown";
}

std::expected<PacketType, PacketError> packetTypeFromU8(std::uint8_t raw) noexcept
{
    switch (raw) {
    case static_cast<std::uint8_t>(PacketType::GameData):
        return PacketType::GameData;
    case static_cast<std::uint8_t>(PacketType::Ping):
        return PacketType::Ping;
    case static_cast<std::uint8_t>(PacketType::Pong):
        return PacketType::Pong;
    case static_cast<std::uint8_t>(PacketType::Connect):
        return PacketType::Connect;
    case static_cast<std::uint8_t>(PacketType::ConnectAck):
        return PacketType::ConnectAck;
    case static_cast<std::uint8_t>(PacketType::Heartbeat):
        return PacketType::Heartbeat;
    default:
        return std::unexpected(PacketError::UnknownType);
    }
}

} // namespace younity::network

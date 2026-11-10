/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/PacketType
** File description:
** PacketType header file
*/

#pragma once

#include <cstdint>
#include <expected>
#include <string_view>

#include "younity/network/PacketError.hpp"

namespace younity::network {

/**
 * @brief Engine-level envelope type.
 *
 * The engine only carries the envelope and an opaque payload: game content
 * lives inside GameData bytes and is defined by the game, so any game can
 * reuse the same packet layer. Ping and Pong carry a 4-byte little-endian
 * cookie used to measure round trips. Connect, ConnectAck and Heartbeat
 * carry no payload and open and keep alive a client session: the client
 * sends Connect until the server answers ConnectAck, then the client sends
 * Heartbeat and the server answers each one while the session lives.
 * Timeouts are decided locally from the last packet seen, so they need no
 * packet of their own.
 *
 * Values 0x00 and 0x04-0x7F are rejected for now and reserved for future
 * envelope uses, 0x83-0xFF for future reliable and session control.
 */
enum class PacketType : std::uint8_t {
    GameData = 0x01,
    Ping = 0x02,
    Pong = 0x03,
    Connect = 0x80,
    ConnectAck = 0x81,
    Heartbeat = 0x82
};

/**
 * @brief Get a short human-readable description of a packet type.
 *
 * @param type The type to describe
 *
 * @return The description, e.g. "ping".
 */
[[nodiscard]] std::string_view toString(PacketType type) noexcept;

/**
 * @brief Convert a raw type byte to a packet type.
 *
 * @param raw The byte read from the header
 *
 * @return The type, or PacketError::UnknownType for any reserved value.
 */
[[nodiscard]] std::expected<PacketType, PacketError> packetTypeFromU8(std::uint8_t raw) noexcept;

} // namespace younity::network

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/Packet
** File description:
** Packet header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <vector>

#include "younity/network/PacketError.hpp"
#include "younity/network/PacketType.hpp"
#include "younity/network/Writer.hpp"

namespace younity::network {

/**
 * @brief Magic sent first in every datagram, "Y6" on the wire.
 *
 * Stored little-endian, so the bytes are 0x59 ('Y') then 0x36 ('6').
 * Rejects traffic from other applications quickly, before any parsing.
 */
inline constexpr std::uint16_t PROTOCOL_MAGIC = 0x3659;

/**
 * @brief Wire version, bumped whenever the header layout changes.
 */
inline constexpr std::uint8_t PROTOCOL_VERSION = 1;

/**
 * @brief Header size in bytes: magic u16 + version u8 + type u8 + seq u32.
 */
inline constexpr std::size_t PACKET_HEADER_BYTES = 8;

/**
 * @brief Ping and Pong payload size in bytes: one little-endian u32 cookie.
 */
inline constexpr std::size_t PING_COOKIE_BYTES = 4;

/**
 * @brief A decoded datagram: header fields plus the opaque payload.
 *
 * The sequence number is a per-sender u32 counter wrapping modulo 2^32;
 * this layer only carries it, ordering and loss policy arrive in later
 * issues and must compare it wrap-aware. GameData payload holds game
 * bytes defined by the game, Ping and Pong hold exactly one cookie.
 */
struct Packet
{
    PacketType type = PacketType::GameData;
    std::uint32_t seq = 0;
    std::vector<std::byte> payload;
};

/**
 * @brief Serialize a packet after the current writer content.
 *
 * @param packet The packet to send
 * @param out The writer receiving the datagram bytes
 *
 * @return false without touching the writer when the type payload is
 *         invalid or the datagram would exceed the packet limit.
 */
[[nodiscard]] bool encodePacket(const Packet &packet, Writer &out) noexcept;

/**
 * @brief Parse and strictly validate datagram bytes.
 *
 * Rejects oversized datagrams, short headers, bad magic, bad version,
 * unknown types and Ping/Pong payloads that are not one cookie.
 *
 * @param datagram The received bytes
 *
 * @return The packet, or the reason it was rejected. Never throws for
 *         malformed input; the payload size is bounded by the datagram.
 */
[[nodiscard]] std::expected<Packet, PacketError> decodePacket(std::span<const std::byte> datagram);

} // namespace younity::network

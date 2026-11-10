/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/SessionPacket
** File description:
** Session packet header file, private to the network module
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "younity/network/Datagram.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Packet.hpp"
#include "younity/network/Writer.hpp"

namespace younity::network {

/**
 * @brief Encode one session packet for a peer.
 *
 * @param type The packet type
 * @param to The peer receiving the packet
 * @param seq The sequence number of the packet
 * @param payload The payload, empty for a session packet
 *
 * @return The send request, or nothing if the encoding failed.
 */
[[nodiscard]] inline std::optional<SendRequest> encodeSessionPacket(
    PacketType type, const Endpoint &to, std::uint32_t seq, std::span<const std::byte> payload = {})
{
    Writer out;
    const Packet packet{
        .type = type, .seq = seq, .payload = std::vector<std::byte>(payload.begin(), payload.end())};

    if (!encodePacket(packet, out)) {
        return std::nullopt;
    }
    const std::span<const std::byte> data = out.getData();
    return SendRequest{.to = to, .bytes = std::vector<std::byte>(data.begin(), data.end())};
}

} // namespace younity::network

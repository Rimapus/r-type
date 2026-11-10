/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/PacketError
** File description:
** PacketError header file
*/

#pragma once

#include <string_view>

namespace younity::network {

/**
 * @brief Why a packet encode or decode failed.
 */
enum class PacketError {
    Truncated,
    TooLarge,
    BadMagic,
    BadVersion,
    UnknownType,
    InvalidPayload
};

/**
 * @brief Get a short human-readable description of a packet error.
 *
 * @param error The error to describe
 *
 * @return The description, e.g. "bad magic".
 */
[[nodiscard]] std::string_view toString(PacketError error) noexcept;

} // namespace younity::network

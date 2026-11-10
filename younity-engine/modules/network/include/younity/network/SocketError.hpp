/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/SocketError
** File description:
** SocketError header file
*/

#pragma once

#include <string_view>

namespace younity::network {

/**
 * @brief Why a UDP socket operation failed.
 */
enum class SocketError {
    NotOpen,
    OpenFailed,
    BindFailed,
    SendFailed,
    ReceiveFailed,
    InvalidAddress,
    // A datagram larger than the receive buffer, refused as such by the system (Windows)
    TooLarge
};

/**
 * @brief Get a short human-readable description of a socket error.
 *
 * @param error The error to describe
 *
 * @return The description, e.g. "not open".
 */
[[nodiscard]] std::string_view toString(SocketError error) noexcept;

} // namespace younity::network

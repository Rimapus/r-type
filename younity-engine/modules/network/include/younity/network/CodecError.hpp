/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/CodecError
** File description:
** CodecError header file
*/

#pragma once

#include <string_view>

namespace younity::network {

/**
 * @brief Why a binary encode or decode failed.
 */
enum class CodecError {
    Truncated,
    TooLarge,
    InvalidValue
};

/**
 * @brief Get a short human-readable description of a codec error.
 *
 * @param error The error to describe
 *
 * @return The description, e.g. "truncated".
 */
[[nodiscard]] std::string_view toString(CodecError error) noexcept;

} // namespace younity::network

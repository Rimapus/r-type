/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Arguments
** File description:
** Arguments header file
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace jtype {

/**
 * @brief Read a UDP port given on the command line.
 *
 * @param text The argument
 *
 * @return The port, or nothing if the argument is not a number from 1 to 65535.
 */
[[nodiscard]] std::optional<std::uint16_t> parsePort(std::string_view text);

} // namespace jtype

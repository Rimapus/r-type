/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Input
** File description:
** Input header file
*/

#pragma once

#include <cstdint>

namespace jtype {

/**
 * @brief The bits of the input bitmask a client sends, one per action held down.
 */
namespace input {

inline constexpr std::uint8_t UP = 1U << 0U;
inline constexpr std::uint8_t DOWN = 1U << 1U;
inline constexpr std::uint8_t LEFT = 1U << 2U;
inline constexpr std::uint8_t RIGHT = 1U << 3U;
inline constexpr std::uint8_t SHOOT = 1U << 4U;

} // namespace input

} // namespace jtype

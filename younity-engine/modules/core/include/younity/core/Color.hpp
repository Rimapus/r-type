/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/Color
** File description:
** Color header file
*/

#pragma once

#include <cstdint>

namespace younity {

/**
 * @brief A color with its opacity, each part from 0 to 255, e.g. Color{255, 0, 0} for an opaque red.
 */
struct Color
{
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    // 0 is invisible, 255 is opaque
    std::uint8_t a = 255;

    bool operator==(const Color &other) const = default;
};

} // namespace younity

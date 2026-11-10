/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Screen
** File description:
** Screen header file
*/

#pragma once

#include <array>

#include "jtype/common/Constants.hpp"
#include "younity/core/Color.hpp"
#include "younity/ecs/components/Transform.hpp"

namespace jtype {

// The game area is drawn this many times bigger, so its pixel art stays sharp
inline constexpr float SCALE = 2.0F;

// The color of each player, the one of its ship and its shots, light enough to tint a text: red, blue, green, yellow
inline constexpr std::array<younity::Color, MAX_PLAYERS> PLAYER_COLORS{
    younity::Color{255, 96,  96 },
    younity::Color{96,  160, 255},
    younity::Color{96,  224, 96 },
    younity::Color{255, 224, 64 }
};

/**
 * @brief Place an entity of the game area in the window.
 *
 * @param x The position in the game area
 * @param y The position in the game area
 *
 * @return The transform drawing it in the window.
 */
[[nodiscard]] inline younity::ecs::Transform toScreen(float x, float y)
{
    younity::ecs::Transform transform;

    transform.position = {x * SCALE, y * SCALE};
    transform.scale = {SCALE, SCALE};
    return transform;
}

} // namespace jtype

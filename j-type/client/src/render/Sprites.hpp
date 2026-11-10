/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Sprites
** File description:
** Sprites header file
*/

#pragma once

#include <array>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"

namespace jtype {

/**
 * @brief A layer of the starfield.
 */
struct StarfieldLayer
{
    const char *path;
    // The part of the scroll speed the layer moves at, the one of its .json file
    float parallax;
};

// The speed the ship flies through space, in pixels of the game area per second
inline constexpr float SCROLL_SPEED = 250.0F;

/**
 * @brief A strip of the ceiling or of the floor, drawn over the starfield and scrolled with the stage.
 */
struct TerrainLayer
{
    const char *path;
    // The top of the strip, in pixels of the game area
    float top;
};

// The ceiling and the floor, under the game entities: the projectiles are on the layer 0
inline constexpr int TERRAIN_LAYER = -2;
inline constexpr std::array<TerrainLayer, 2> TERRAIN_LAYERS{
    {
     {"sprites/16bit/backgrounds/fg_ceiling.png", 0.0F},
     {"sprites/16bit/backgrounds/fg_floor.png", WORLD_HEIGHT - TERRAIN_HEIGHT},
     }
};

// From the farthest to the closest
inline constexpr std::array<StarfieldLayer, 2> STARFIELD_LAYERS{
    {
     {"sprites/16bit/backgrounds/bg0_space.png", 0.05F},
     {"sprites/16bit/backgrounds/bg1_stars.png", 0.12F},
     }
};

} // namespace jtype

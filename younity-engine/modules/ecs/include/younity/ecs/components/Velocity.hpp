/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/components/Velocity
** File description:
** Velocity header file
*/

#pragma once

#include "younity/core/Vector2.hpp"

namespace younity::ecs {

/**
 * @brief How fast and in which direction an entity moves.
 */
struct Velocity
{
    // In pixels per second
    Vector2f value{};
};

} // namespace younity::ecs

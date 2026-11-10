/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/components/Hitbox
** File description:
** Hitbox header file
*/

#pragma once

#include "younity/core/Vector2.hpp"

namespace younity::physics {

/**
 * @brief Rectangle used to detect the collisions of an entity, placed from its Transform.
 */
struct Hitbox
{
    Vector2f size{};
    Vector2f offset{};
};

} // namespace younity::physics

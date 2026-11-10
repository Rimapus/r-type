/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/CollisionEvent
** File description:
** CollisionEvent header file
*/

#pragma once

#include "younity/ecs/Entity.hpp"

namespace younity::physics {

/**
 * @brief Published by the CollisionSystem for each pair of entities whose hitboxes overlap,
 *        once per frame as long as they overlap.
 */
struct CollisionEvent
{
    ecs::Entity first;
    ecs::Entity second;
};

} // namespace younity::physics

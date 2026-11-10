/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/components/Gravity
** File description:
** Gravity header file
*/

#pragma once

namespace younity::physics {

/**
 * @brief Makes an entity fall: the GravitySystem adds the gravity of the world to its Velocity.
 */
struct Gravity
{
    // Multiplies the gravity of the world: 0 floats, 2 falls twice as fast, -1 rises
    float scale{1.0F};
};

} // namespace younity::physics

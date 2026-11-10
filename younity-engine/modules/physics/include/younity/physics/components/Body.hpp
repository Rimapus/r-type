/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/components/Body
** File description:
** Body header file
*/

#pragma once

namespace younity::physics {

/**
 * @brief Makes an entity stop on the solids, slide along them and bounce on them.
 */
struct Body
{
    // Share of the speed sent back when hitting a solid: 0 stops, 1 bounces back at full speed
    float bounce{0.0F};
    // Set by the SolidCollisionSystem: true when a solid pushed the body up during the last step
    bool onGround{false};
};

} // namespace younity::physics

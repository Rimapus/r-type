/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/systems/SolidCollisionSystem
** File description:
** SolidCollisionSystem header file
*/

#pragma once

#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace younity::physics {

/**
 * @brief Pushes the bodies out of the solids they overlap, so they stop, slide and bounce.
 *        Add it after the MovementSystem with registry.addSystem<SolidCollisionSystem, Transform, Velocity, Hitbox, Body>().
 */
class SolidCollisionSystem : public ecs::System
{
public:
    /**
     * @brief Push each body out of the solids it overlaps, along the axis where it overlaps the least.
     *        On that axis, the velocity going into the solid becomes -velocity * bounce, and the other axis is kept.
     *        Body::onGround is true when a solid pushed the body up.
     *
     * @param registry The registry owning the entities, the solids are its entities with a Transform, a Hitbox and a Solid
     * @param entities The entities having a Transform, a Velocity, a Hitbox and a Body
     * @param dt The time elapsed since the last frame, in seconds (delta time)
     */
    void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override;
};

} // namespace younity::physics

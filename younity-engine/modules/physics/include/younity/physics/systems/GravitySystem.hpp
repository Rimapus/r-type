/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/systems/GravitySystem
** File description:
** GravitySystem header file
*/

#pragma once

#include <vector>

#include "younity/core/Vector2.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace younity::physics {

/**
 * @brief Accelerates the entities having a Gravity by the gravity of the world.
 *        Add it before the MovementSystem with registry.addSystem<GravitySystem, Velocity, Gravity>(gravity).
 */
class GravitySystem : public ecs::System
{
public:
    /**
     * @brief Create the system.
     *
     * @param gravity The gravity of the world, in pixels per second squared, e.g. {0, 980} to fall down the screen
     */
    explicit GravitySystem(Vector2f gravity);

    /**
     * @brief Add the gravity of the world, multiplied by the scale of each entity and the delta time, to its velocity.
     *
     * @param registry The registry owning the entities
     * @param entities The entities having a Velocity and a Gravity
     * @param dt The time elapsed since the last frame, in seconds (delta time)
     */
    void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override;

private:
    Vector2f _gravity;
};

} // namespace younity::physics

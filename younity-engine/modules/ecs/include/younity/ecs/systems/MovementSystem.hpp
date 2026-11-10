/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/systems/MovementSystem
** File description:
** MovementSystem header file
*/

#pragma once

#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace younity::ecs {

/**
 * @brief Moves the entities by their velocity.
 *        Add it with registry.addSystem<MovementSystem, Transform, Velocity>().
 */
class MovementSystem : public System
{
public:
    /**
     * @brief Move each entity by its velocity multiplied by the delta time.
     *
     * @param registry The registry owning the entities
     * @param entities The entities having a Transform and a Velocity
     * @param dt The time elapsed since the last frame, in seconds (delta time)
     */
    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override;
};

} // namespace younity::ecs

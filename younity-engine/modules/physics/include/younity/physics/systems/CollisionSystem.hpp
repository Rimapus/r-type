/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/systems/CollisionSystem
** File description:
** CollisionSystem header file
*/

#pragma once

#include <vector>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace younity::physics {

/**
 * @brief Detects the entities whose hitboxes overlap (AABB) and publishes a CollisionEvent for each pair.
 *        Add it with registry.addSystem<CollisionSystem, Transform, Hitbox>(bus).
 */
class CollisionSystem : public ecs::System
{
public:
    /**
     * @brief Create the system.
     *
     * @param bus The event bus the collisions are published on
     */
    explicit CollisionSystem(EventBus &bus);

    /**
     * @brief Publish a CollisionEvent for each pair of entities whose hitboxes overlap.
     *        The scale of the Transform is applied to the hitbox, its rotation is ignored.
     *
     * @param registry The registry owning the entities
     * @param entities The entities having a Transform and a Hitbox
     * @param dt The time elapsed since the last frame, in seconds (delta time)
     */
    void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override;

private:
    EventBus &_bus;
};

} // namespace younity::physics

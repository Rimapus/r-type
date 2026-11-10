/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/systems/LifetimeSystem
** File description:
** LifetimeSystem header file
*/

#pragma once

#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace jtype {

/**
 * @brief Destroys the short-lived entities, e.g. the explosions, once their Lifetime is over.
 *        Add it with registry.addSystem<LifetimeSystem, Lifetime>().
 */
class LifetimeSystem : public younity::ecs::System
{
public:
    /**
     * @brief Count down the lifetime of each entity, and destroy the ones that reached 0.
     *
     * @param registry The registry owning the entities
     * @param entities The entities having a Lifetime
     * @param dt The duration of the step, in seconds
     */
    void update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt) override;
};

} // namespace jtype

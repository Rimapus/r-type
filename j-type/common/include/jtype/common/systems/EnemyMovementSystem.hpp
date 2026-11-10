/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/systems/EnemyMovementSystem
** File description:
** EnemyMovementSystem header file
*/

#pragma once

#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace jtype {

/**
 * @brief Steers the Bydos by their MovementPattern, at BYDO_SPEED pixels per second: Straight to the left,
 *        SineWave to the left while waving up and down, Chase towards the nearest player for CHASE_DURATION, then
 *        to the left.
 *        Add it with registry.addSystem<EnemyMovementSystem, Enemy, Transform, Velocity>(), before the MovementSystem.
 */
class EnemyMovementSystem : public younity::ecs::System
{
public:
    /**
     * @brief Set the velocity of each Bydo for this step.
     *
     * @param registry The registry owning the Bydos and the players
     * @param entities The entities having an Enemy, a Transform and a Velocity
     * @param dt The duration of the step, in seconds
     */
    void update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt) override;
};

} // namespace jtype

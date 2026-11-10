/*
** EPITECH PROJECT, 2026
** j-type/common/src/systems/EnemyMovementSystem
** File description:
** EnemyMovementSystem source file
*/

#include "jtype/common/systems/EnemyMovementSystem.hpp"

#include <cmath>
#include <numbers>
#include <optional>

#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Targeting.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"

namespace jtype {

namespace {

/**
 * @brief Get the velocity of a Bydo chasing the nearest player.
 *
 * @param registry The registry owning the players
 * @param position Where the Bydo is
 *
 * @return Towards the player, or to the left when no player can be chased.
 */
younity::Vector2f chase(const younity::ecs::Registry &registry, younity::Vector2f position)
{
    const std::optional<younity::Vector2f> target = findNearestPlayer(registry, position);

    if (!target || *target == position) {
        return {-BYDO_SPEED, 0.0F};
    }
    return (*target - position).normalized() * BYDO_SPEED;
}

} // namespace

void EnemyMovementSystem::update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt)
{
    for (const younity::ecs::Entity entity : entities) {
        Enemy &enemy = registry.getComponent<Enemy>(entity);
        younity::Vector2f &velocity = registry.getComponent<younity::ecs::Velocity>(entity).value;

        enemy.age += dt;
        switch (enemy.pattern) {
        case MovementPattern::Straight:
            velocity = {-BYDO_SPEED, 0.0F};
            break;
        case MovementPattern::SineWave: {
            // The derivative of SINE_AMPLITUDE * sin(pulsation * age): the height follows a sine
            const float pulsation = 2.0F * std::numbers::pi_v<float> * SINE_FREQUENCY;

            velocity = {-BYDO_SPEED, SINE_AMPLITUDE * pulsation * std::cos(pulsation * enemy.age)};
            break;
        }
        case MovementPattern::Chase:
            // A chaser never left the screen: past its time it flies away, so its wave can end
            if (enemy.age >= CHASE_DURATION) {
                velocity = {-BYDO_SPEED, 0.0F};
            } else {
                velocity = chase(registry, registry.getComponent<younity::ecs::Transform>(entity).position);
            }
            break;
        }
    }
}

} // namespace jtype

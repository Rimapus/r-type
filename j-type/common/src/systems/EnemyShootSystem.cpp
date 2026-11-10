/*
** EPITECH PROJECT, 2026
** j-type/common/src/systems/EnemyShootSystem
** File description:
** EnemyShootSystem source file
*/

#include "jtype/common/systems/EnemyShootSystem.hpp"

#include <optional>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/Targeting.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"

namespace jtype {

EnemyShootSystem::EnemyShootSystem(const DifficultyRules &rules)
    : _rules(rules)
{
}

void EnemyShootSystem::update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt)
{
    for (const younity::ecs::Entity entity : entities) {
        Enemy &enemy = registry.getComponent<Enemy>(entity);
        const younity::Vector2f position = registry.getComponent<younity::ecs::Transform>(entity).position;

        // Off-screen the time before the first shot does not run: it starts when the Bydo comes in
        if (!canShoot(enemy.type) || position.x - BYDO_ORIGIN.x >= WORLD_WIDTH) {
            continue;
        }
        enemy.shootCooldown -= dt;
        if (enemy.shootCooldown > 0.0F) {
            continue;
        }
        enemy.shootCooldown = ENEMY_SHOOT_DELAY * this->_rules.shootDelay;

        younity::Vector2f direction{-1.0F, 0.0F};

        if (enemy.type == EnemyType::SentryDrone) {
            const std::optional<younity::Vector2f> target = findNearestPlayer(registry, position);

            // A player right on the drone gives no direction: it shoots straight
            if (target && *target != position) {
                direction = (*target - position).normalized();
            }
        }
        spawnEnemyMissile(registry, position, direction * ENEMY_MISSILE_SPEED * this->_rules.shotSpeed);
    }
}

} // namespace jtype

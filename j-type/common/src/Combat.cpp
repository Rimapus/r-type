/*
** EPITECH PROJECT, 2026
** j-type/common/src/Combat
** File description:
** Combat source file
*/

#include "jtype/common/Combat.hpp"

#include <algorithm>
#include <utility>

#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Health.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Obstacle.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/components/PowerUp.hpp"
#include "jtype/common/components/Score.hpp"
#include "younity/ecs/components/Transform.hpp"

namespace jtype {

namespace {

/**
 * @brief Use up a missile: it is destroyed at the end of the step, and loses its Missile at once,
 *        so another collision of the same step cannot use it again.
 *
 * @param registry The registry of the game
 * @param missile The missile
 */
void consume(younity::ecs::Registry &registry, younity::ecs::Entity missile)
{
    registry.removeComponent<Missile>(missile);
    registry.destroyEntity(missile);
}

/**
 * @brief Add points to the score of a player.
 *
 * @param registry The registry of the game
 * @param slot The slot of the player
 * @param points The points earned
 */
void givePoints(younity::ecs::Registry &registry, std::uint8_t slot, std::uint32_t points)
{
    for (const younity::ecs::Entity ship : registry.getEntitiesWith<Player, Score>()) {
        if (registry.getComponent<Player>(ship).slot == slot) {
            registry.getComponent<Score>(ship).points += points;
        }
    }
}

/**
 * @brief Destroy every part of the boss, and leave its wreck where the body was.
 *
 * @param registry The registry of the game
 */
void destroyBoss(younity::ecs::Registry &registry)
{
    for (const younity::ecs::Entity part : registry.getEntitiesWith<Boss>()) {
        if (registry.getComponent<Boss>(part).part == BossPart::Body) {
            spawnBossWreck(registry, registry.getComponent<younity::ecs::Transform>(part).position);
        }
        // Removed at once, so the other collisions of the step leave the wreck alone
        registry.removeComponent<Boss>(part);
        registry.destroyEntity(part);
    }
}

/**
 * @brief Give a power-up to a ship.
 *
 * @param registry The registry of the game
 * @param ship The ship that took it
 * @param type What it gives
 */
void givePowerUp(younity::ecs::Registry &registry, younity::ecs::Entity ship, PowerUpType type)
{
    Player &player = registry.getComponent<Player>(ship);

    switch (type) {
    case PowerUpType::ExtraLife:
        player.lives = std::min<std::uint8_t>(player.lives + 1, MAX_LIVES);
        break;
    case PowerUpType::Shield:
        registry.getComponent<Health>(ship).current = SHIELDED_HEALTH;
        break;
    case PowerUpType::SpreadShot:
        player.weapon = Weapon::Spread;
        break;
    }
}

/**
 * @brief Apply what one entity of a pair does to the other one.
 *
 * @param registry The registry of the game
 * @param target The entity that can be hurt
 * @param source The entity that can hurt it
 * @param deaths Where the destroyed entities are reported
 * @param pickUps Where the power-ups taken are reported
 */
void strike(younity::ecs::Registry &registry, younity::ecs::Entity target, younity::ecs::Entity source,
            std::vector<Death> *deaths, std::vector<PickUp> *pickUps)
{
    const bool isMissile = registry.hasComponent<Missile>(source);
    const bool fromPlayer = isMissile && registry.getComponent<Missile>(source).shooter.has_value();

    if (registry.hasComponent<Obstacle>(target) && isMissile) {
        const Missile missile = registry.getComponent<Missile>(source);

        // Every obstacle stops the missiles, and only a block of a wall breaks under the ones of the players
        consume(registry, source);
        if (missile.shooter && registry.hasComponent<Health>(target)) {
            hurt(registry, target, missile.damage, missile.shooter, deaths);
        }
    } else if (registry.hasComponent<Boss>(target) && fromPlayer) {
        const Missile missile = registry.getComponent<Missile>(source);

        // The body is armored: it stops the missile, and only the eye is hurt
        consume(registry, source);
        if (registry.getComponent<Boss>(target).part == BossPart::Eye) {
            hurt(registry, target, missile.damage, missile.shooter, deaths);
        }
    } else if (registry.hasComponent<Enemy>(target) && fromPlayer) {
        const Missile missile = registry.getComponent<Missile>(source);

        consume(registry, source);
        hurt(registry, target, missile.damage, missile.shooter, deaths);
    } else if (registry.hasComponent<Player>(target) && isMissile && !fromPlayer) {
        const int damage = registry.getComponent<Missile>(source).damage;

        consume(registry, source);
        hurt(registry, target, damage, std::nullopt, deaths);
    } else if (registry.hasComponent<Player>(target) && registry.hasComponent<Enemy>(source)) {
        hurt(registry, target, 1, std::nullopt, deaths);
        // The Bydo crashes into the ship: it is destroyed whatever its health
        hurt(registry, source, registry.getComponent<Health>(source).current, std::nullopt, deaths);
    } else if (registry.hasComponent<Player>(target) && (registry.hasComponent<Boss>(source) || registry.hasComponent<Obstacle>(source))) {
        // The boss and the obstacles stay: only the ship is hurt
        hurt(registry, target, 1, std::nullopt, deaths);
    } else if (registry.hasComponent<Player>(target) && registry.hasComponent<PowerUp>(source)) {
        const PowerUpType type = registry.getComponent<PowerUp>(source).type;

        // Removed at once, so another ship touching it in the same step does not take it too
        registry.removeComponent<PowerUp>(source);
        registry.destroyEntity(source);
        givePowerUp(registry, target, type);
        if (pickUps) {
            pickUps->push_back(PickUp{registry.getComponent<Player>(target).slot, type});
        }
    }
}

} // namespace

void hurt(younity::ecs::Registry &registry, younity::ecs::Entity entity, int damage, std::optional<std::uint8_t> shooter,
          std::vector<Death> *deaths)
{
    if (registry.hasComponent<Player>(entity) && registry.getComponent<Player>(entity).invulnerable > 0.0F) {
        return;
    }
    Health &health = registry.getComponent<Health>(entity);

    health.current -= damage;
    if (health.current > 0) {
        return;
    }
    health.current = health.max;

    // Copied before the explosion is created: adding its Transform may move the others
    const younity::Vector2f position = registry.getComponent<younity::ecs::Transform>(entity).position;

    spawnExplosion(registry, position);
    if (deaths) {
        deaths->push_back(Death{entity, shooter});
    }
    if (registry.hasComponent<Player>(entity)) {
        Player &player = registry.getComponent<Player>(entity);

        player.lives = player.lives > 0 ? player.lives - 1 : 0;
        if (player.lives == 0) {
            // Out of the game: removed at once, so the other collisions of the step leave the wreck alone
            registry.removeComponent<Player>(entity);
            registry.destroyEntity(entity);
            return;
        }
        // Left where it was: the player keeps steering it, and a jump back to the start would fly it into the ceiling
        player.invulnerable = RESPAWN_INVULNERABILITY;
        player.weapon = Weapon::Plasma;
    } else if (registry.hasComponent<Boss>(entity)) {
        if (shooter) {
            givePoints(registry, *shooter, BOSS_POINTS);
        }
        destroyBoss(registry);
    } else if (registry.hasComponent<Obstacle>(entity)) {
        // A broken block of a wall: removed at once, so the other collisions of the step leave the wreck alone
        registry.removeComponent<Obstacle>(entity);
        registry.destroyEntity(entity);
    } else {
        const Enemy enemy = registry.getComponent<Enemy>(entity);

        // A Bydo that crashes into a ship leaves nothing: only a shot earns the points and the power-up
        if (shooter) {
            givePoints(registry, *shooter, getKillPoints(enemy.type));
            if (enemy.drop) {
                spawnPowerUp(registry, position, *enemy.drop);
            }
        }
        // Removed at once, so the other collisions of the step leave the wreck alone
        registry.removeComponent<Enemy>(entity);
        registry.destroyEntity(entity);
    }
}

void resolveCollision(younity::ecs::Registry &registry, const younity::physics::CollisionEvent &collision,
                      std::vector<Death> *deaths, std::vector<PickUp> *pickUps)
{
    // Each pair comes once, in any order: look at it both ways
    strike(registry, collision.first, collision.second, deaths, pickUps);
    strike(registry, collision.second, collision.first, deaths, pickUps);
}

} // namespace jtype

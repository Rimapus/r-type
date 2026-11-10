/*
** EPITECH PROJECT, 2026
** j-type/common/src/systems/BossSystem
** File description:
** BossSystem source file
*/

#include "jtype/common/systems/BossSystem.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/Targeting.hpp"
#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Health.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"

namespace jtype {

namespace {

// Where the body stops: its right side on the right edge
constexpr float REST_X = WORLD_WIDTH - (BOSS_SIZE.x / 2.0F);

/**
 * @brief Fire an orb from the mouth at the nearest player.
 *
 * @param registry The registry owning the players
 * @param mouth Where the orb leaves
 * @param speed The speed of the orb
 */
void fireOrb(younity::ecs::Registry &registry, younity::Vector2f mouth, float speed)
{
    const std::optional<younity::Vector2f> target = findNearestPlayer(registry, mouth);
    younity::Vector2f direction{-1.0F, 0.0F};

    // Without a player, or with a player right on the mouth, it shoots straight
    if (target && *target != mouth) {
        direction = (*target - mouth).normalized();
    }
    spawnEnemyMissile(registry, mouth, direction * speed, MissileKind::BossOrb);
}

/**
 * @brief Fire a fan of spits from the mouth, around the left.
 *
 * @param registry The registry the spits are created in
 * @param mouth Where the spits leave
 * @param speed The speed of each spit
 */
void fireSpits(younity::ecs::Registry &registry, younity::Vector2f mouth, float speed)
{
    for (int spit = 0; spit < BOSS_SPIT_COUNT; ++spit) {
        const float angle = (static_cast<float>(spit) - (static_cast<float>(BOSS_SPIT_COUNT - 1) / 2.0F)) * BOSS_SPIT_SPREAD;

        spawnEnemyMissile(registry, mouth, younity::Vector2f{-std::cos(angle), std::sin(angle)} * speed,
                          MissileKind::BossSpit);
    }
}

} // namespace

BossSystem::BossSystem(const DifficultyRules &rules)
    : _rules(rules)
{
}

void BossSystem::update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt)
{
    std::optional<younity::ecs::Entity> body;
    std::optional<younity::ecs::Entity> eye;

    for (const younity::ecs::Entity entity : entities) {
        if (registry.getComponent<Boss>(entity).part == BossPart::Body) {
            body = entity;
        } else {
            eye = entity;
        }
    }
    if (!body) {
        return;
    }
    Boss &boss = registry.getComponent<Boss>(*body);
    younity::Vector2f &position = registry.getComponent<younity::ecs::Transform>(*body).position;
    bool angry = false;

    if (eye) {
        const Health &health = registry.getComponent<Health>(*eye);

        angry = health.current * 2 <= health.max;
    }
    if (position.x > REST_X) {
        position.x = std::max(position.x - (BOSS_ENTER_SPEED * dt), REST_X);
    } else {
        const float pulsation = 2.0F * std::numbers::pi_v<float> * BOSS_RISE_FREQUENCY;

        boss.age += dt;
        position.y = (WORLD_HEIGHT / 2.0F) + (BOSS_RISE_AMPLITUDE * std::sin(pulsation * boss.age));
        boss.orbCooldown -= dt;
        boss.spitCooldown -= dt;
    }
    // Copied: the missiles add a Transform, which may move the one of the body
    const younity::Vector2f placed = position;

    if (eye) {
        registry.getComponent<younity::ecs::Transform>(*eye).position = placed + BOSS_EYE_OFFSET;
    }
    if (boss.orbCooldown <= 0.0F) {
        boss.orbCooldown = angry ? BOSS_ANGRY_ORB_DELAY : BOSS_ORB_DELAY;
        fireOrb(registry, placed + BOSS_MOUTH_OFFSET, ENEMY_MISSILE_SPEED * this->_rules.shotSpeed);
    }
    if (boss.spitCooldown <= 0.0F) {
        boss.spitCooldown = angry ? BOSS_ANGRY_SPIT_DELAY : BOSS_SPIT_DELAY;
        fireSpits(registry, placed + BOSS_MOUTH_OFFSET, ENEMY_MISSILE_SPEED * this->_rules.shotSpeed);
    }
}

} // namespace jtype

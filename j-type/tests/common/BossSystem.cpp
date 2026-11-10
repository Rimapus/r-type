/*
** EPITECH PROJECT, 2026
** j-type/tests/common/BossSystem
** File description:
** BossSystem unit tests
*/

#include <criterion/criterion.h>

#include <algorithm>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Health.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/systems/BossSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"

using jtype::Boss;
using jtype::BossPart;
using jtype::BossSystem;
using jtype::Health;
using jtype::Missile;
using jtype::MissileKind;
using jtype::Player;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::ecs::Velocity;

namespace {

// Where the body stops: its right side on the right edge
const float REST_X = jtype::WORLD_WIDTH - (jtype::BOSS_SIZE.x / 2.0F);
// Long enough for the body to come in from past the right edge
const float ENTER_TIME = jtype::BOSS_SIZE.x / jtype::BOSS_ENTER_SPEED;

// The incubator of one player, moved by the BossSystem
Entity createBoss(Registry &registry)
{
    registry.addSystem<BossSystem, Boss, Transform>();
    return jtype::spawnIncubator(registry, 1);
}

Entity findEye(const Registry &registry)
{
    for (const Entity part : registry.getEntitiesWith<Boss>()) {
        if (registry.getComponent<Boss>(part).part == BossPart::Eye) {
            return part;
        }
    }
    cr_assert_fail("the boss has no eye");
    return Entity(0);
}

std::vector<Entity> shotsOf(const Registry &registry, MissileKind kind)
{
    std::vector<Entity> shots;

    for (const Entity missile : registry.getEntitiesWith<Missile>()) {
        if (registry.getComponent<Missile>(missile).kind == kind) {
            shots.push_back(missile);
        }
    }
    return shots;
}

} // namespace

Test(common_boss_system, comes_in_then_stops_at_the_right_edge)
{
    Registry registry;
    const Entity body = createBoss(registry);

    cr_assert_gt(registry.getComponent<Transform>(body).position.x, jtype::WORLD_WIDTH);
    registry.updateSystems(1.0F);
    cr_assert_float_eq(registry.getComponent<Transform>(body).position.x,
                       jtype::WORLD_WIDTH + (jtype::BOSS_SIZE.x / 2.0F) - jtype::BOSS_ENTER_SPEED, 1e-3F);
    registry.updateSystems(ENTER_TIME);
    cr_assert_float_eq(registry.getComponent<Transform>(body).position.x, REST_X, 1e-3F);
}

Test(common_boss_system, rises_and_falls_between_the_ceiling_and_the_floor)
{
    Registry registry;
    const Entity body = createBoss(registry);

    registry.updateSystems(ENTER_TIME);
    for (int step = 0; step < 200; ++step) {
        const float y = registry.getComponent<Transform>(body).position.y;

        cr_assert_geq(y - (jtype::BOSS_SIZE.y / 2.0F), jtype::TERRAIN_HEIGHT);
        cr_assert_leq(y + (jtype::BOSS_SIZE.y / 2.0F), jtype::WORLD_HEIGHT - jtype::TERRAIN_HEIGHT);
        registry.updateSystems(0.1F);
    }
}

Test(common_boss_system, eye_follows_the_body)
{
    Registry registry;
    const Entity body = createBoss(registry);

    // At the edge, while it rises or falls
    registry.updateSystems(ENTER_TIME);
    registry.updateSystems(2.5F);
    cr_assert(registry.getComponent<Transform>(findEye(registry)).position ==
              registry.getComponent<Transform>(body).position + jtype::BOSS_EYE_OFFSET);
}

Test(common_boss_system, does_not_fire_while_it_comes_in)
{
    Registry registry;

    createBoss(registry);
    registry.updateSystems(ENTER_TIME / 2.0F);
    cr_assert(registry.getEntitiesWith<Missile>().empty());
}

Test(common_boss_system, fires_an_orb_at_the_nearest_player)
{
    Registry registry;
    const Entity ship = registry.createEntity();
    const Entity body = createBoss(registry);

    registry.addComponent(ship, Player{});
    registry.addComponent(ship, Transform{
                                    Vector2f{60.0F, 40.0F}
    });
    registry.updateSystems(ENTER_TIME);
    registry.updateSystems(jtype::BOSS_ORB_DELAY);

    const std::vector<Entity> orbs = shotsOf(registry, MissileKind::BossOrb);

    cr_assert_eq(orbs.size(), 1U);

    const Vector2f mouth = registry.getComponent<Transform>(orbs[0]).position;
    const Vector2f toShip = (Vector2f{60.0F, 40.0F} - mouth).normalized() * jtype::ENEMY_MISSILE_SPEED;
    const Vector2f velocity = registry.getComponent<Velocity>(orbs[0]).value;

    cr_assert(mouth == registry.getComponent<Transform>(body).position + jtype::BOSS_MOUTH_OFFSET);
    cr_assert_float_eq(velocity.x, toShip.x, 1e-3F);
    cr_assert_float_eq(velocity.y, toShip.y, 1e-3F);
}

Test(common_boss_system, fires_a_fan_of_spits_to_the_left)
{
    Registry registry;

    createBoss(registry);
    registry.updateSystems(ENTER_TIME);
    registry.updateSystems(jtype::BOSS_SPIT_DELAY / 2.0F);

    const std::vector<Entity> spits = shotsOf(registry, MissileKind::BossSpit);
    float lowest = 0.0F;
    float highest = 0.0F;

    cr_assert_eq(spits.size(), static_cast<std::size_t>(jtype::BOSS_SPIT_COUNT));
    for (const Entity spit : spits) {
        const Vector2f velocity = registry.getComponent<Velocity>(spit).value;

        cr_assert_lt(velocity.x, 0.0F);
        lowest = std::min(lowest, velocity.y);
        highest = std::max(highest, velocity.y);
    }
    // Spread up and down around the left
    cr_assert_float_eq(lowest, -highest, 1e-3F);
    cr_assert_gt(highest, 0.0F);
}

Test(common_boss_system, fires_faster_once_the_eye_lost_half_of_its_health)
{
    Registry registry;

    createBoss(registry);
    registry.updateSystems(ENTER_TIME);
    registry.updateSystems(jtype::BOSS_ORB_DELAY);
    cr_assert_eq(shotsOf(registry, MissileKind::BossOrb).size(), 1U);

    Health &health = registry.getComponent<Health>(findEye(registry));

    health.current = health.max / 2;
    registry.updateSystems(jtype::BOSS_ORB_DELAY);
    registry.updateSystems(jtype::BOSS_ANGRY_ORB_DELAY);
    cr_assert_eq(shotsOf(registry, MissileKind::BossOrb).size(), 3U);
}

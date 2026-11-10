/*
** EPITECH PROJECT, 2026
** j-type/tests/common/Combat
** File description:
** Unit tests of the combat rules
*/

#include <criterion/criterion.h>

#include <vector>

#include "jtype/common/Combat.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Explosion.hpp"
#include "jtype/common/components/Health.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Obstacle.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/components/PowerUp.hpp"
#include "jtype/common/components/Score.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/physics/CollisionEvent.hpp"

using jtype::Boss;
using jtype::BossPart;
using jtype::BossWreck;
using jtype::Death;
using jtype::Enemy;
using jtype::EnemyType;
using jtype::Explosion;
using jtype::Health;
using jtype::Missile;
using jtype::PickUp;
using jtype::Player;
using jtype::PowerUp;
using jtype::PowerUpType;
using jtype::Score;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::physics::CollisionEvent;

namespace {

const Vector2f PLACE{200.0F, 100.0F};

// The ship of player 1, moved away from its start position
Entity createShip(Registry &registry)
{
    const Entity ship = jtype::spawnPlayer(registry, 0);

    registry.getComponent<Transform>(ship).position = PLACE;
    return ship;
}

Entity createEnemyMissile(Registry &registry)
{
    return jtype::spawnEnemyMissile(registry, PLACE, Vector2f{});
}

std::size_t countExplosions(const Registry &registry)
{
    return registry.getEntitiesWith<Explosion>().size();
}

// Counted, not checked by id: outside a step, a destroyed entity gives its id to the next one, e.g. the explosion
std::size_t countMissiles(const Registry &registry)
{
    return registry.getEntitiesWith<Missile>().size();
}

std::size_t countBydos(const Registry &registry)
{
    return registry.getEntitiesWith<Enemy>().size();
}

// The part of the incubator of one player, by default its eye
Entity findBossPart(const Registry &registry, BossPart part = BossPart::Eye)
{
    for (const Entity entity : registry.getEntitiesWith<Boss>()) {
        if (registry.getComponent<Boss>(entity).part == part) {
            return entity;
        }
    }
    cr_assert_fail("the boss has no such part");
    return Entity(0);
}

} // namespace

Test(common_combat, missile_of_a_player_destroys_a_blobling)
{
    Registry registry;
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::Blobling);
    const Entity missile = jtype::spawnMissile(registry, PLACE, 0);

    jtype::resolveCollision(registry, CollisionEvent{missile, bydo});
    cr_assert_eq(countBydos(registry), 0U);
    cr_assert_eq(countMissiles(registry), 0U);
    cr_assert_eq(countExplosions(registry), 1U);
}

Test(common_combat, destroyed_bydo_is_reported_with_its_killer)
{
    Registry registry;
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::Blobling);
    const Entity missile = jtype::spawnMissile(registry, PLACE, 2);
    std::vector<Death> deaths;

    jtype::resolveCollision(registry, CollisionEvent{missile, bydo}, &deaths);
    cr_assert_eq(deaths.size(), 1U);
    cr_assert_eq(deaths[0].entity.getId(), bydo.getId());
    cr_assert(deaths[0].killer.has_value());
    cr_assert_eq(*deaths[0].killer, 2);
}

Test(common_combat, missile_of_a_player_damages_a_jelly)
{
    Registry registry;
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::Jelly);
    const Entity missile = jtype::spawnMissile(registry, PLACE, 0);

    jtype::resolveCollision(registry, CollisionEvent{bydo, missile});
    cr_assert_eq(registry.getComponent<Health>(bydo).current, 1);
    cr_assert_eq(countMissiles(registry), 0U);
}

Test(common_combat, missile_hurts_one_target_only)
{
    Registry registry;
    const Entity first = jtype::spawnBydo(registry, PLACE, EnemyType::Jelly);
    const Entity second = jtype::spawnBydo(registry, PLACE, EnemyType::Jelly);
    const Entity missile = jtype::spawnMissile(registry, PLACE, 0);

    jtype::resolveCollision(registry, CollisionEvent{missile, first});
    jtype::resolveCollision(registry, CollisionEvent{missile, second});
    cr_assert_eq(registry.getComponent<Health>(first).current, 1);
    cr_assert_eq(registry.getComponent<Health>(second).current, 2);
}

Test(common_combat, missile_of_a_bydo_leaves_the_bydos_alone)
{
    Registry registry;
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::Blobling);
    const Entity missile = createEnemyMissile(registry);

    jtype::resolveCollision(registry, CollisionEvent{missile, bydo});
    cr_assert_eq(countBydos(registry), 1U);
    cr_assert_eq(countMissiles(registry), 1U);
}

Test(common_combat, missile_of_a_bydo_destroys_a_ship_that_comes_back_where_it_was)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity missile = createEnemyMissile(registry);

    jtype::resolveCollision(registry, CollisionEvent{ship, missile});
    cr_assert(registry.getComponent<Transform>(ship).position == PLACE);
    cr_assert_eq(registry.getComponent<Health>(ship).current, jtype::PLAYER_HEALTH);
    cr_assert_float_eq(registry.getComponent<Player>(ship).invulnerable, jtype::RESPAWN_INVULNERABILITY, 1e-6F);
    cr_assert_eq(countMissiles(registry), 0U);
    cr_assert_eq(countExplosions(registry), 1U);
}

Test(common_combat, missile_of_a_player_leaves_the_ships_alone)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity missile = jtype::spawnMissile(registry, PLACE, 1);

    jtype::resolveCollision(registry, CollisionEvent{missile, ship});
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES);
    cr_assert(registry.hasComponent<Missile>(missile));
}

Test(common_combat, bydo_crashes_into_a_ship)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::SentryDrone);

    jtype::resolveCollision(registry, CollisionEvent{bydo, ship});
    cr_assert_eq(countBydos(registry), 0U);
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES - 1);
    cr_assert_eq(countExplosions(registry), 2U);
}

Test(common_combat, crash_reports_no_killer)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::SentryDrone);
    std::vector<Death> deaths;

    jtype::resolveCollision(registry, CollisionEvent{bydo, ship}, &deaths);
    // The crash destroys the ship, which comes back, then the Bydo
    cr_assert_eq(deaths.size(), 2U);
    cr_assert_eq(deaths[0].entity.getId(), ship.getId());
    cr_assert_not(deaths[0].killer.has_value());
    cr_assert_eq(deaths[1].entity.getId(), bydo.getId());
    cr_assert_not(deaths[1].killer.has_value());
}

Test(common_combat, invulnerable_ship_is_not_hurt)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity missile = createEnemyMissile(registry);

    registry.getComponent<Player>(ship).invulnerable = 1.0F;
    jtype::resolveCollision(registry, CollisionEvent{ship, missile});
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES);
    cr_assert_eq(countExplosions(registry), 0U);
}

Test(common_combat, health_depends_on_the_type)
{
    cr_assert_eq(jtype::getMaxHealth(EnemyType::Blobling), 1);
    cr_assert_eq(jtype::getMaxHealth(EnemyType::Jelly), 2);
    cr_assert_eq(jtype::getMaxHealth(EnemyType::SentryDrone), 3);
}

Test(common_combat, player_earns_the_points_of_the_bydo)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity bydo = jtype::spawnBydo(registry, Vector2f{400.0F, 100.0F}, EnemyType::SentryDrone);
    const Entity missile = jtype::spawnMissile(registry, Vector2f{400.0F, 100.0F}, 0);

    registry.getComponent<Health>(bydo).current = 1;
    jtype::resolveCollision(registry, CollisionEvent{missile, bydo});
    cr_assert_eq(registry.getComponent<Score>(ship).points, jtype::getKillPoints(EnemyType::SentryDrone));
}

Test(common_combat, crash_gives_no_points)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::Blobling);

    jtype::resolveCollision(registry, CollisionEvent{bydo, ship});
    cr_assert_eq(registry.getComponent<Score>(ship).points, 0U);
}

Test(common_combat, destroyed_ship_loses_a_life)
{
    Registry registry;
    const Entity ship = createShip(registry);

    jtype::hurt(registry, ship, 1);
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES - 1);
}

Test(common_combat, last_life_takes_the_ship_out_of_the_game)
{
    Registry registry;
    const Entity ship = createShip(registry);

    registry.getComponent<Player>(ship).lives = 1;
    jtype::hurt(registry, ship, 1);
    cr_assert(registry.getEntitiesWith<Player>().empty());
    cr_assert_eq(countExplosions(registry), 1U);
}

Test(common_combat, ship_out_of_lives_is_reported)
{
    Registry registry;
    const Entity ship = createShip(registry);
    std::vector<Death> deaths;

    registry.getComponent<Player>(ship).lives = 1;
    jtype::hurt(registry, ship, 1, std::nullopt, &deaths);
    cr_assert_eq(deaths.size(), 1U);
    cr_assert_eq(deaths[0].entity.getId(), ship.getId());
    cr_assert_not(deaths[0].killer.has_value());
}

Test(common_combat, kill_points_depend_on_the_type)
{
    cr_assert_eq(jtype::getKillPoints(EnemyType::Blobling), 100U);
    cr_assert_eq(jtype::getKillPoints(EnemyType::Jelly), 200U);
    cr_assert_eq(jtype::getKillPoints(EnemyType::SentryDrone), 300U);
}

Test(common_combat, body_of_the_boss_stops_the_missiles)
{
    Registry registry;

    jtype::spawnIncubator(registry, 1);

    const Entity missile = jtype::spawnMissile(registry, PLACE, 0);

    jtype::resolveCollision(registry, CollisionEvent{missile, findBossPart(registry, BossPart::Body)});
    cr_assert_eq(countMissiles(registry), 0U);
    cr_assert_eq(registry.getComponent<Health>(findBossPart(registry)).current, jtype::BOSS_HEALTH);
}

Test(common_combat, eye_of_the_boss_takes_the_missiles)
{
    Registry registry;

    jtype::spawnIncubator(registry, 1);

    const Entity missile = jtype::spawnMissile(registry, PLACE, 0);

    jtype::resolveCollision(registry, CollisionEvent{findBossPart(registry), missile});
    cr_assert_eq(countMissiles(registry), 0U);
    cr_assert_eq(registry.getComponent<Health>(findBossPart(registry)).current, jtype::BOSS_HEALTH - 1);
}

Test(common_combat, destroyed_eye_takes_the_boss_with_it)
{
    Registry registry;
    const Entity ship = createShip(registry);

    jtype::spawnIncubator(registry, 1);
    registry.getComponent<Health>(findBossPart(registry)).current = 1;

    const Entity missile = jtype::spawnMissile(registry, PLACE, 0);
    std::vector<Death> deaths;

    jtype::resolveCollision(registry, CollisionEvent{missile, findBossPart(registry)}, &deaths);
    cr_assert(registry.getEntitiesWith<Boss>().empty());
    cr_assert_eq(registry.getEntitiesWith<BossWreck>().size(), 1U);
    cr_assert_eq(registry.getComponent<Score>(ship).points, jtype::BOSS_POINTS);
    cr_assert_eq(deaths.size(), 1U);
}

Test(common_combat, boss_hurts_the_ship_it_touches_and_stays)
{
    Registry registry;
    const Entity ship = createShip(registry);

    jtype::spawnIncubator(registry, 1);
    jtype::resolveCollision(registry, CollisionEvent{findBossPart(registry, BossPart::Body), ship});
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES - 1);
    cr_assert_eq(registry.getEntitiesWith<Boss>().size(), 2U);
}

Test(common_combat, missile_of_the_boss_leaves_the_boss_alone)
{
    Registry registry;

    jtype::spawnIncubator(registry, 1);

    const Entity orb = jtype::spawnEnemyMissile(registry, PLACE, Vector2f{}, jtype::MissileKind::BossOrb);

    jtype::resolveCollision(registry, CollisionEvent{orb, findBossPart(registry)});
    cr_assert_eq(countMissiles(registry), 1U);
    cr_assert_eq(registry.getComponent<Health>(findBossPart(registry)).current, jtype::BOSS_HEALTH);
}

Test(common_combat, shot_carrier_leaves_its_power_up)
{
    Registry registry;
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::Blobling);

    registry.getComponent<Enemy>(bydo).drop = PowerUpType::Shield;
    jtype::resolveCollision(registry, CollisionEvent{jtype::spawnMissile(registry, PLACE, 0), bydo});

    const std::vector<Entity> powerUps = registry.getEntitiesWith<PowerUp>();

    cr_assert_eq(powerUps.size(), 1U);
    cr_assert(registry.getComponent<PowerUp>(powerUps[0]).type == PowerUpType::Shield);
    cr_assert(registry.getComponent<Transform>(powerUps[0]).position == PLACE);
}

Test(common_combat, carrier_crashing_into_a_ship_leaves_nothing)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity bydo = jtype::spawnBydo(registry, PLACE, EnemyType::Blobling);

    registry.getComponent<Enemy>(bydo).drop = PowerUpType::Shield;
    jtype::resolveCollision(registry, CollisionEvent{ship, bydo});
    cr_assert(registry.getEntitiesWith<PowerUp>().empty());
}

Test(common_combat, ship_takes_a_power_up_and_it_is_reported)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity powerUp = jtype::spawnPowerUp(registry, PLACE, PowerUpType::SpreadShot);
    std::vector<PickUp> pickUps;

    jtype::resolveCollision(registry, CollisionEvent{powerUp, ship}, nullptr, &pickUps);
    cr_assert(registry.getComponent<Player>(ship).weapon == jtype::Weapon::Spread);
    cr_assert(registry.getEntitiesWith<PowerUp>().empty());
    cr_assert_eq(pickUps.size(), 1U);
    cr_assert_eq(pickUps[0].slot, 0);
    cr_assert(pickUps[0].type == PowerUpType::SpreadShot);
}

Test(common_combat, extra_life_stops_at_the_most_lives)
{
    Registry registry;
    const Entity ship = createShip(registry);

    for (int taken = 0; taken < jtype::MAX_LIVES; ++taken) {
        jtype::resolveCollision(registry, CollisionEvent{ship, jtype::spawnPowerUp(registry, PLACE, PowerUpType::ExtraLife)});
    }
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::MAX_LIVES);
}

Test(common_combat, shield_takes_the_next_hit)
{
    Registry registry;
    const Entity ship = createShip(registry);

    jtype::resolveCollision(registry, CollisionEvent{ship, jtype::spawnPowerUp(registry, PLACE, PowerUpType::Shield)});
    cr_assert_eq(registry.getComponent<Health>(ship).current, jtype::SHIELDED_HEALTH);
    jtype::resolveCollision(registry, CollisionEvent{createEnemyMissile(registry), ship});
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES);
    cr_assert_eq(registry.getComponent<Health>(ship).current, jtype::PLAYER_HEALTH);
    jtype::resolveCollision(registry, CollisionEvent{createEnemyMissile(registry), ship});
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES - 1);
}

Test(common_combat, ship_loses_its_weapon_with_a_life)
{
    Registry registry;
    const Entity ship = createShip(registry);

    registry.getComponent<Player>(ship).weapon = jtype::Weapon::Spread;
    jtype::resolveCollision(registry, CollisionEvent{createEnemyMissile(registry), ship});
    cr_assert(registry.getComponent<Player>(ship).weapon == jtype::Weapon::Plasma);
}

Test(common_combat, block_breaks_under_the_missiles_of_the_players)
{
    Registry registry;
    const Entity block = jtype::spawnWall(registry, 0.5F, 0.5F).front();

    jtype::resolveCollision(registry, CollisionEvent{jtype::spawnMissile(registry, PLACE, 0), block});
    cr_assert_eq(registry.getComponent<Health>(block).current, jtype::BLOCK_HEALTH - 1);
    cr_assert_eq(countMissiles(registry), 0U);
    jtype::resolveCollision(registry, CollisionEvent{block, jtype::spawnMissile(registry, PLACE, 0)});
    cr_assert(registry.getEntitiesWith<jtype::Obstacle>().empty());
    cr_assert_eq(countExplosions(registry), 1U);
}

Test(common_combat, zapper_stops_every_missile_and_never_breaks)
{
    Registry registry;
    const Entity node = jtype::spawnZapper(registry, 0.2F, 0.6F).front();

    jtype::resolveCollision(registry, CollisionEvent{jtype::spawnMissile(registry, PLACE, 0), node});
    jtype::resolveCollision(registry, CollisionEvent{createEnemyMissile(registry), node});
    cr_assert_eq(countMissiles(registry), 0U);
    cr_assert(registry.isAlive(node));
}

Test(common_combat, obstacle_hurts_the_ship_it_touches_and_stays)
{
    Registry registry;
    const Entity ship = createShip(registry);
    const Entity beam = jtype::spawnZapper(registry, 0.2F, 0.6F)[1];

    jtype::resolveCollision(registry, CollisionEvent{beam, ship});
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES - 1);
    cr_assert(registry.isAlive(beam));
}

Test(common_combat, ship_that_touches_the_floor_loses_a_life)
{
    Registry registry;
    const Entity ship = createShip(registry);

    jtype::spawnTerrain(registry);

    const std::vector<Entity> terrain = registry.getEntitiesWith<jtype::Obstacle>();

    jtype::resolveCollision(registry, CollisionEvent{ship, terrain.back()});
    cr_assert_eq(registry.getComponent<Player>(ship).lives, jtype::PLAYER_LIVES - 1);
    cr_assert_eq(registry.getEntitiesWith<jtype::Obstacle>().size(), 2U);
}

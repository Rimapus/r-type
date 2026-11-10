/*
** EPITECH PROJECT, 2026
** j-type/tests/common/EnemyShootSystem
** File description:
** EnemyShootSystem unit tests
*/

#include <criterion/criterion.h>

#include <cmath>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Difficulty.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/systems/EnemyShootSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"

using jtype::Enemy;
using jtype::EnemyShootSystem;
using jtype::EnemyType;
using jtype::Missile;
using jtype::MovementPattern;
using jtype::Player;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::ecs::Velocity;

namespace {

// A Bydo at (300, 100), about to shoot
Entity createBydo(Registry &registry, EnemyType type)
{
    const Entity bydo = registry.createEntity();

    registry.addSystem<EnemyShootSystem, Enemy, Transform>();
    registry.addComponent(bydo, Transform{
                                    Vector2f{300.0F, 100.0F}
    });
    registry.addComponent(bydo, Enemy{type, MovementPattern::Straight, 0.0F});
    return bydo;
}

void createPlayer(Registry &registry, Vector2f position)
{
    const Entity ship = registry.createEntity();

    registry.addComponent(ship, Transform{position});
    registry.addComponent(ship, Player{});
}

std::vector<Entity> missiles(const Registry &registry)
{
    return registry.getEntitiesWith<Missile>();
}

} // namespace

Test(common_enemy_shoot_system, shoots_straight_to_the_left)
{
    Registry registry;

    createBydo(registry, EnemyType::Jelly);
    createPlayer(registry, Vector2f{100.0F, 200.0F});
    registry.updateSystems(0.01F);
    cr_assert_eq(missiles(registry).size(), 1U);
    cr_assert(registry.getComponent<Velocity>(missiles(registry)[0]).value == (Vector2f{-jtype::ENEMY_MISSILE_SPEED, 0.0F}));
}

Test(common_enemy_shoot_system, missile_has_no_shooter)
{
    Registry registry;

    createBydo(registry, EnemyType::Jelly);
    registry.updateSystems(0.01F);
    cr_assert_not(registry.getComponent<Missile>(missiles(registry)[0]).shooter.has_value());
}

Test(common_enemy_shoot_system, sentry_drone_aims_at_the_nearest_player)
{
    Registry registry;

    createBydo(registry, EnemyType::SentryDrone);
    createPlayer(registry, Vector2f{300.0F, 400.0F});
    createPlayer(registry, Vector2f{300.0F, 50.0F});
    registry.updateSystems(0.01F);

    const Vector2f velocity = registry.getComponent<Velocity>(missiles(registry)[0]).value;

    cr_assert_float_eq(velocity.x, 0.0F, 1e-3F);
    cr_assert_float_eq(velocity.y, -jtype::ENEMY_MISSILE_SPEED, 1e-3F);
}

Test(common_enemy_shoot_system, sentry_drone_without_player_shoots_straight)
{
    Registry registry;

    createBydo(registry, EnemyType::SentryDrone);
    registry.updateSystems(0.01F);
    cr_assert(registry.getComponent<Velocity>(missiles(registry)[0]).value == (Vector2f{-jtype::ENEMY_MISSILE_SPEED, 0.0F}));
}

Test(common_enemy_shoot_system, waits_between_two_shots)
{
    Registry registry;

    createBydo(registry, EnemyType::Jelly);
    registry.updateSystems(0.01F);
    registry.updateSystems(jtype::ENEMY_SHOOT_DELAY / 2.0F);
    cr_assert_eq(missiles(registry).size(), 1U);
    registry.updateSystems(jtype::ENEMY_SHOOT_DELAY / 2.0F);
    cr_assert_eq(missiles(registry).size(), 2U);
}

Test(common_enemy_shoot_system, blobling_does_not_shoot)
{
    Registry registry;

    createBydo(registry, EnemyType::Blobling);
    registry.updateSystems(jtype::ENEMY_SHOOT_DELAY * 3.0F);
    cr_assert(missiles(registry).empty());
}

Test(common_enemy_shoot_system, first_shot_waits_until_it_is_on_the_screen)
{
    Registry registry;

    registry.addSystem<EnemyShootSystem, Enemy, Transform>();

    const Entity bydo = jtype::spawnBydo(registry, Vector2f{jtype::WORLD_WIDTH + 30.0F, 100.0F}, EnemyType::Jelly);

    // Off-screen, the time does not count
    registry.updateSystems(jtype::ENEMY_SHOOT_DELAY * 3.0F);
    cr_assert(missiles(registry).empty());
    registry.getComponent<Transform>(bydo).position.x = 300.0F;
    registry.updateSystems(jtype::ENEMY_FIRST_SHOT_DELAY / 2.0F);
    cr_assert(missiles(registry).empty());
    registry.updateSystems(jtype::ENEMY_FIRST_SHOT_DELAY / 2.0F);
    cr_assert_eq(missiles(registry).size(), 1U);
}

Test(common_enemy_shoot_system, shoots_slower_and_less_often_in_easy)
{
    const jtype::DifficultyRules easy = jtype::getDifficultyRules(jtype::Difficulty::Easy);
    Registry registry;
    const Entity bydo = registry.createEntity();

    registry.addSystem<EnemyShootSystem, Enemy, Transform>(easy);
    registry.addComponent(bydo, Transform{
                                    Vector2f{300.0F, 100.0F}
    });
    registry.addComponent(bydo, Enemy{EnemyType::Jelly, MovementPattern::Straight, 0.0F});
    registry.updateSystems(0.01F);
    cr_assert_eq(missiles(registry).size(), 1U);
    cr_assert_float_eq(registry.getComponent<Velocity>(missiles(registry)[0]).value.x,
                       -jtype::ENEMY_MISSILE_SPEED * easy.shotSpeed, 1e-3F);
    // A normal Bydo shoots again after ENEMY_SHOOT_DELAY, this one later
    registry.updateSystems(jtype::ENEMY_SHOOT_DELAY);
    cr_assert_eq(missiles(registry).size(), 1U);
    registry.updateSystems(jtype::ENEMY_SHOOT_DELAY * (easy.shootDelay - 1.0F));
    cr_assert_eq(missiles(registry).size(), 2U);
}

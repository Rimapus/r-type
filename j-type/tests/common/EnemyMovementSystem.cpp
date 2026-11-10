/*
** EPITECH PROJECT, 2026
** j-type/tests/common/EnemyMovementSystem
** File description:
** EnemyMovementSystem unit tests
*/

#include <criterion/criterion.h>

#include <cmath>

#include "jtype/common/Gameplay.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/systems/EnemyMovementSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"

using jtype::Enemy;
using jtype::EnemyMovementSystem;
using jtype::EnemyType;
using jtype::MovementPattern;
using jtype::Player;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::ecs::Velocity;

namespace {

// A Bydo at (300, 100) moving with the given pattern
Entity createBydo(Registry &registry, MovementPattern pattern)
{
    const Entity bydo = registry.createEntity();

    registry.addSystem<EnemyMovementSystem, Enemy, Transform, Velocity>();
    registry.addComponent(bydo, Transform{
                                    Vector2f{300.0F, 100.0F}
    });
    registry.addComponent(bydo, Velocity{});
    registry.addComponent(bydo, Enemy{EnemyType::Blobling, pattern});
    return bydo;
}

Vector2f velocityOf(const Registry &registry, Entity bydo)
{
    return registry.getComponent<Velocity>(bydo).value;
}

} // namespace

Test(common_enemy_movement_system, straight_goes_left)
{
    Registry registry;
    const Entity bydo = createBydo(registry, MovementPattern::Straight);

    registry.updateSystems(0.1F);
    cr_assert(velocityOf(registry, bydo) == (Vector2f{-jtype::BYDO_SPEED, 0.0F}));
}

Test(common_enemy_movement_system, sine_wave_goes_left_while_going_up_and_down)
{
    Registry registry;
    const Entity bydo = createBydo(registry, MovementPattern::SineWave);

    // At the start it goes down the screen, half a wave later it goes up
    registry.updateSystems(0.0F);
    cr_assert_gt(velocityOf(registry, bydo).y, 0.0F);
    registry.updateSystems(0.5F / jtype::SINE_FREQUENCY);
    cr_assert_lt(velocityOf(registry, bydo).y, 0.0F);
    cr_assert_float_eq(velocityOf(registry, bydo).x, -jtype::BYDO_SPEED, 1e-4F);
}

Test(common_enemy_movement_system, chase_goes_to_the_nearest_player)
{
    Registry registry;
    const Entity bydo = createBydo(registry, MovementPattern::Chase);
    const Entity near = registry.createEntity();
    const Entity far = registry.createEntity();

    registry.addComponent(near, Transform{
                                    Vector2f{300.0F, 200.0F}
    });
    registry.addComponent(near, Player{});
    registry.addComponent(far, Transform{
                                   Vector2f{0.0F, 100.0F}
    });
    registry.addComponent(far, Player{});
    registry.updateSystems(0.1F);
    cr_assert_float_eq(velocityOf(registry, bydo).x, 0.0F, 1e-4F);
    cr_assert_float_eq(velocityOf(registry, bydo).y, jtype::BYDO_SPEED, 1e-4F);
}

Test(common_enemy_movement_system, chase_without_player_goes_left)
{
    Registry registry;
    const Entity bydo = createBydo(registry, MovementPattern::Chase);

    registry.updateSystems(0.1F);
    cr_assert(velocityOf(registry, bydo) == (Vector2f{-jtype::BYDO_SPEED, 0.0F}));
}

Test(common_enemy_movement_system, chase_flies_away_after_its_time)
{
    Registry registry;
    const Entity bydo = createBydo(registry, MovementPattern::Chase);
    const Entity ship = registry.createEntity();

    registry.addComponent(ship, Transform{
                                    Vector2f{300.0F, 200.0F}
    });
    registry.addComponent(ship, Player{});
    registry.updateSystems(jtype::CHASE_DURATION);
    cr_assert(velocityOf(registry, bydo) == (Vector2f{-jtype::BYDO_SPEED, 0.0F}));
}

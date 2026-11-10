/*
** EPITECH PROJECT, 2026
** j-type/tests/common/BoundsSystem
** File description:
** BoundsSystem unit tests
*/

#include <criterion/criterion.h>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Obstacle.hpp"
#include "jtype/common/systems/BoundsSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"

using jtype::BoundsSystem;
using jtype::Enemy;
using jtype::Missile;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;

namespace {

Entity createMissile(Registry &registry, Vector2f position)
{
    const Entity missile = registry.createEntity();

    registry.addComponent(missile, Transform{position});
    registry.addComponent(missile, Missile{});
    return missile;
}

} // namespace

Test(common_bounds_system, destroys_the_missiles_past_any_edge)
{
    Registry registry;

    registry.addSystem<BoundsSystem, Transform>();

    const Entity right = createMissile(registry, Vector2f{jtype::WORLD_WIDTH + 50.0F, 100.0F});
    const Entity left = createMissile(registry, Vector2f{-50.0F, 100.0F});
    const Entity top = createMissile(registry, Vector2f{100.0F, -50.0F});
    const Entity bottom = createMissile(registry, Vector2f{100.0F, jtype::WORLD_HEIGHT + 50.0F});
    const Entity inside = createMissile(registry, Vector2f{100.0F, 100.0F});

    registry.updateSystems(0.01F);
    cr_assert_not(registry.isAlive(right));
    cr_assert_not(registry.isAlive(left));
    cr_assert_not(registry.isAlive(top));
    cr_assert_not(registry.isAlive(bottom));
    cr_assert(registry.isAlive(inside));
}

Test(common_bounds_system, destroys_the_bydos_past_the_left_edge)
{
    Registry registry;

    registry.addSystem<BoundsSystem, Transform>();

    const Entity gone = registry.createEntity();
    const Entity coming = registry.createEntity();

    registry.addComponent(gone, Transform{
                                    Vector2f{-100.0F, 100.0F}
    });
    registry.addComponent(gone, Enemy{});
    // Bydos appear past the right edge: they must stay
    registry.addComponent(coming, Transform{
                                      Vector2f{jtype::WORLD_WIDTH + 100.0F, 100.0F}
    });
    registry.addComponent(coming, Enemy{});
    registry.updateSystems(0.01F);
    cr_assert_not(registry.isAlive(gone));
    cr_assert(registry.isAlive(coming));
}

Test(common_bounds_system, destroys_the_power_ups_gone_on_the_left)
{
    Registry registry;

    registry.addSystem<BoundsSystem, Transform>();

    const Entity gone = jtype::spawnPowerUp(registry, Vector2f{-20.0F, 100.0F}, jtype::PowerUpType::Shield);
    const Entity coming = jtype::spawnPowerUp(registry, Vector2f{jtype::WORLD_WIDTH + 20.0F, 100.0F}, jtype::PowerUpType::Shield);

    registry.updateSystems(0.0F);
    cr_assert_not(registry.isAlive(gone));
    cr_assert(registry.isAlive(coming));
}

Test(common_bounds_system, destroys_the_walls_gone_on_the_left_and_keeps_the_terrain)
{
    Registry registry;

    registry.addSystem<BoundsSystem, Transform>();
    jtype::spawnTerrain(registry);

    const Entity block = jtype::spawnWall(registry, 0.5F, 0.5F).front();

    registry.getComponent<Transform>(block).position.x = -20.0F;
    registry.updateSystems(0.0F);
    cr_assert_not(registry.isAlive(block));
    cr_assert_eq(registry.getEntitiesWith<jtype::Obstacle>().size(), 2U);
}

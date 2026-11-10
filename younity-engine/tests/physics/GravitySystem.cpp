/*
** EPITECH PROJECT, 2026
** younity-engine/tests/physics/GravitySystem
** File description:
** GravitySystem unit tests
*/

#include <criterion/criterion.h>

#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/ecs/systems/MovementSystem.hpp"
#include "younity/physics/components/Gravity.hpp"
#include "younity/physics/systems/GravitySystem.hpp"

using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::MovementSystem;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::ecs::Velocity;
using younity::physics::Gravity;
using younity::physics::GravitySystem;

namespace {

// A falling entity, with a Velocity and a Gravity of the given scale
Entity createFalling(Registry &registry, float scale)
{
    const Entity entity = registry.createEntity();
    Gravity gravity;

    gravity.scale = scale;
    registry.addComponent(entity, Velocity{});
    registry.addComponent(entity, gravity);
    return entity;
}

} // namespace

Test(physics_gravity_system, accelerates_by_gravity_times_dt)
{
    Registry registry;
    const Entity entity = createFalling(registry, 1.0F);

    registry.addSystem<GravitySystem, Velocity, Gravity>(Vector2f{0.0F, 100.0F});
    registry.updateSystems(0.5F);
    cr_assert(registry.getComponent<Velocity>(entity).value == (Vector2f{0.0F, 50.0F}));
}

Test(physics_gravity_system, accelerates_again_each_frame)
{
    Registry registry;
    const Entity entity = createFalling(registry, 1.0F);

    registry.addSystem<GravitySystem, Velocity, Gravity>(Vector2f{0.0F, 100.0F});
    registry.updateSystems(1.0F);
    registry.updateSystems(1.0F);
    cr_assert(registry.getComponent<Velocity>(entity).value == (Vector2f{0.0F, 200.0F}));
}

Test(physics_gravity_system, scale_multiplies_the_gravity)
{
    Registry registry;
    const Entity heavy = createFalling(registry, 2.0F);
    const Entity floating = createFalling(registry, 0.0F);

    registry.addSystem<GravitySystem, Velocity, Gravity>(Vector2f{0.0F, 100.0F});
    registry.updateSystems(1.0F);
    cr_assert(registry.getComponent<Velocity>(heavy).value == (Vector2f{0.0F, 200.0F}));
    cr_assert(registry.getComponent<Velocity>(floating).value == (Vector2f{0.0F, 0.0F}));
}

Test(physics_gravity_system, entity_without_gravity_keeps_its_velocity)
{
    Registry registry;
    const Entity entity = registry.createEntity();
    Velocity velocity;

    velocity.value = Vector2f{10.0F, 0.0F};
    registry.addSystem<GravitySystem, Velocity, Gravity>(Vector2f{0.0F, 100.0F});
    registry.addComponent(entity, velocity);
    registry.updateSystems(1.0F);
    cr_assert(registry.getComponent<Velocity>(entity).value == (Vector2f{10.0F, 0.0F}));
}

Test(physics_gravity_system, falls_with_the_movement_system)
{
    Registry registry;
    const Entity entity = createFalling(registry, 1.0F);

    registry.addSystem<GravitySystem, Velocity, Gravity>(Vector2f{0.0F, 100.0F});
    registry.addSystem<MovementSystem, Transform, Velocity>();
    registry.addComponent(entity, Transform{});
    registry.updateSystems(1.0F);
    registry.updateSystems(1.0F);
    cr_assert(registry.getComponent<Transform>(entity).position == (Vector2f{0.0F, 300.0F}));
}

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/MovementSystem
** File description:
** MovementSystem unit tests
*/

#include <criterion/criterion.h>

#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/ecs/systems/MovementSystem.hpp"

using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::MovementSystem;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::ecs::Velocity;

Test(ecs_movement_system, moves_by_velocity_times_dt)
{
    Registry registry;
    const Entity entity = registry.createEntity();
    Velocity velocity;

    velocity.value = Vector2f{10.0F, -4.0F};
    registry.addSystem<MovementSystem, Transform, Velocity>();
    registry.addComponent(entity, Transform{});
    registry.addComponent(entity, velocity);
    registry.updateSystems(0.5F);
    cr_assert(registry.getComponent<Transform>(entity).position == (Vector2f{5.0F, -2.0F}));
}

Test(ecs_movement_system, moves_again_each_frame)
{
    Registry registry;
    const Entity entity = registry.createEntity();
    Velocity velocity;

    velocity.value = Vector2f{10.0F, 0.0F};
    registry.addSystem<MovementSystem, Transform, Velocity>();
    registry.addComponent(entity, Transform{});
    registry.addComponent(entity, velocity);
    registry.updateSystems(1.0F);
    registry.updateSystems(1.0F);
    cr_assert(registry.getComponent<Transform>(entity).position == (Vector2f{20.0F, 0.0F}));
}

Test(ecs_movement_system, entity_without_velocity_does_not_move)
{
    Registry registry;
    const Entity entity = registry.createEntity();
    Transform transform;

    transform.position = Vector2f{3.0F, 4.0F};
    registry.addSystem<MovementSystem, Transform, Velocity>();
    registry.addComponent(entity, transform);
    registry.updateSystems(1.0F);
    cr_assert(registry.getComponent<Transform>(entity).position == (Vector2f{3.0F, 4.0F}));
}

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/physics/SolidCollisionSystem
** File description:
** SolidCollisionSystem unit tests
*/

#include <criterion/criterion.h>

#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/ecs/systems/MovementSystem.hpp"
#include "younity/physics/components/Body.hpp"
#include "younity/physics/components/Gravity.hpp"
#include "younity/physics/components/Hitbox.hpp"
#include "younity/physics/components/Solid.hpp"
#include "younity/physics/systems/GravitySystem.hpp"
#include "younity/physics/systems/SolidCollisionSystem.hpp"

using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::MovementSystem;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::ecs::Velocity;
using younity::physics::Body;
using younity::physics::Gravity;
using younity::physics::GravitySystem;
using younity::physics::Hitbox;
using younity::physics::Solid;
using younity::physics::SolidCollisionSystem;

namespace {

// A registry with a SolidCollisionSystem
struct World
{
    World(void)
    {
        this->registry.addSystem<SolidCollisionSystem, Transform, Velocity, Hitbox, Body>();
    }

    Registry registry;
};

// An entity with a hitbox of the given size, at the given position
Entity createBox(Registry &registry, Vector2f position, Vector2f size)
{
    const Entity entity = registry.createEntity();
    Transform transform;
    Hitbox hitbox;

    transform.position = position;
    hitbox.size = size;
    registry.addComponent(entity, transform);
    registry.addComponent(entity, hitbox);
    return entity;
}

// A 10 x 10 body at the given position, moving at the given speed
Entity createBody(Registry &registry, Vector2f position, Vector2f speed, float bounce = 0.0F)
{
    const Entity entity = createBox(registry, position, Vector2f{10.0F, 10.0F});
    Velocity velocity;
    Body body;

    velocity.value = speed;
    body.bounce = bounce;
    registry.addComponent(entity, velocity);
    registry.addComponent(entity, body);
    return entity;
}

// A solid ground whose top is at y = 10
Entity createGround(Registry &registry)
{
    const Entity entity = createBox(registry, Vector2f{-100.0F, 10.0F}, Vector2f{200.0F, 100.0F});

    registry.addComponent(entity, Solid{});
    return entity;
}

} // namespace

Test(physics_solid_collision_system, stops_on_the_ground)
{
    World world;
    const Entity body = createBody(world.registry, Vector2f{0.0F, 5.0F}, Vector2f{0.0F, 100.0F});

    createGround(world.registry);
    world.registry.updateSystems(0.0F);
    cr_assert(world.registry.getComponent<Transform>(body).position == (Vector2f{0.0F, 0.0F}));
    cr_assert(world.registry.getComponent<Velocity>(body).value == (Vector2f{0.0F, 0.0F}));
    cr_assert(world.registry.getComponent<Body>(body).onGround);
}

Test(physics_solid_collision_system, slides_along_the_ground)
{
    World world;
    const Entity body = createBody(world.registry, Vector2f{0.0F, 5.0F}, Vector2f{50.0F, 100.0F});

    createGround(world.registry);
    world.registry.updateSystems(0.0F);
    cr_assert(world.registry.getComponent<Velocity>(body).value == (Vector2f{50.0F, 0.0F}));
}

Test(physics_solid_collision_system, bounce_sends_the_speed_back)
{
    World world;
    const Entity body = createBody(world.registry, Vector2f{0.0F, 5.0F}, Vector2f{0.0F, 100.0F}, 0.5F);

    createGround(world.registry);
    world.registry.updateSystems(0.0F);
    cr_assert(world.registry.getComponent<Velocity>(body).value == (Vector2f{0.0F, -50.0F}));
}

Test(physics_solid_collision_system, stops_on_a_wall)
{
    World world;
    const Entity body = createBody(world.registry, Vector2f{5.0F, 0.0F}, Vector2f{100.0F, 0.0F});
    const Entity wall = createBox(world.registry, Vector2f{10.0F, -50.0F}, Vector2f{10.0F, 100.0F});

    world.registry.addComponent(wall, Solid{});
    world.registry.updateSystems(0.0F);
    cr_assert(world.registry.getComponent<Transform>(body).position == (Vector2f{0.0F, 0.0F}));
    cr_assert(world.registry.getComponent<Velocity>(body).value == (Vector2f{0.0F, 0.0F}));
    cr_assert_not(world.registry.getComponent<Body>(body).onGround);
}

Test(physics_solid_collision_system, moving_out_keeps_its_speed)
{
    World world;
    const Entity body = createBody(world.registry, Vector2f{0.0F, 5.0F}, Vector2f{0.0F, -100.0F});

    createGround(world.registry);
    world.registry.updateSystems(0.0F);
    cr_assert(world.registry.getComponent<Velocity>(body).value == (Vector2f{0.0F, -100.0F}));
}

Test(physics_solid_collision_system, leaves_the_ground_when_apart)
{
    World world;
    const Entity body = createBody(world.registry, Vector2f{0.0F, 5.0F}, Vector2f{0.0F, 100.0F});

    createGround(world.registry);
    world.registry.updateSystems(0.0F);
    world.registry.getComponent<Transform>(body).position = Vector2f{0.0F, -50.0F};
    world.registry.updateSystems(0.0F);
    cr_assert_not(world.registry.getComponent<Body>(body).onGround);
}

Test(physics_solid_collision_system, body_without_solid_is_left_in_place)
{
    World world;
    const Entity body = createBody(world.registry, Vector2f{0.0F, 5.0F}, Vector2f{0.0F, 100.0F});

    createBox(world.registry, Vector2f{-100.0F, 10.0F}, Vector2f{200.0F, 100.0F});
    world.registry.updateSystems(0.0F);
    cr_assert(world.registry.getComponent<Transform>(body).position == (Vector2f{0.0F, 5.0F}));
}

Test(physics_solid_collision_system, solid_body_ignores_itself)
{
    World world;
    const Entity body = createBody(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{0.0F, 0.0F});

    world.registry.addComponent(body, Solid{});
    world.registry.updateSystems(0.0F);
    cr_assert(world.registry.getComponent<Transform>(body).position == (Vector2f{0.0F, 0.0F}));
}

Test(physics_solid_collision_system, falls_and_lands_with_gravity_and_movement)
{
    Registry registry;
    const Entity body = createBody(registry, Vector2f{0.0F, -20.0F}, Vector2f{0.0F, 0.0F});

    registry.addSystem<GravitySystem, Velocity, Gravity>(Vector2f{0.0F, 64.0F});
    registry.addSystem<MovementSystem, Transform, Velocity>();
    registry.addSystem<SolidCollisionSystem, Transform, Velocity, Hitbox, Body>();
    registry.addComponent(body, Gravity{});
    createGround(registry);
    for (int step = 0; step < 10; ++step) {
        registry.updateSystems(0.125F);
    }
    cr_assert(registry.getComponent<Transform>(body).position == (Vector2f{0.0F, 0.0F}));
    cr_assert(registry.getComponent<Body>(body).onGround);
}

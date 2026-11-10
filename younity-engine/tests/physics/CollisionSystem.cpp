/*
** EPITECH PROJECT, 2026
** younity-engine/tests/physics/CollisionSystem
** File description:
** CollisionSystem unit tests
*/

#include <criterion/criterion.h>

#include <vector>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/physics/CollisionEvent.hpp"
#include "younity/physics/components/Hitbox.hpp"
#include "younity/physics/systems/CollisionSystem.hpp"

using younity::EventBus;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::physics::CollisionEvent;
using younity::physics::CollisionSystem;
using younity::physics::Hitbox;

namespace {

// A registry with a CollisionSystem, keeping the collisions it publishes
struct World
{
    World(void)
    {
        this->bus.subscribe<CollisionEvent>([this](const CollisionEvent &event) {
            this->collisions.push_back(event);
        });
        this->registry.addSystem<CollisionSystem, Transform, Hitbox>(this->bus);
    }

    EventBus bus;
    Registry registry;
    std::vector<CollisionEvent> collisions;
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

} // namespace

Test(physics_collision_system, overlapping_entities_collide)
{
    World world;
    const Entity first = createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    const Entity second = createBox(world.registry, Vector2f{5.0F, 5.0F}, Vector2f{10.0F, 10.0F});

    world.registry.updateSystems(0.0F);
    cr_assert_eq(world.collisions.size(), 1);
    cr_assert(world.collisions[0].first == first);
    cr_assert(world.collisions[0].second == second);
}

Test(physics_collision_system, separated_entities_do_not_collide)
{
    World world;

    createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    createBox(world.registry, Vector2f{50.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    world.registry.updateSystems(0.0F);
    cr_assert(world.collisions.empty());
}

Test(physics_collision_system, entities_touching_by_an_edge_do_not_collide)
{
    World world;

    createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    createBox(world.registry, Vector2f{10.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    world.registry.updateSystems(0.0F);
    cr_assert(world.collisions.empty());
}

Test(physics_collision_system, each_pair_collides_once)
{
    World world;

    createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    createBox(world.registry, Vector2f{2.0F, 2.0F}, Vector2f{10.0F, 10.0F});
    createBox(world.registry, Vector2f{4.0F, 4.0F}, Vector2f{10.0F, 10.0F});
    world.registry.updateSystems(0.0F);
    cr_assert_eq(world.collisions.size(), 3);
}

Test(physics_collision_system, collides_at_each_frame_while_overlapping)
{
    World world;

    createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    createBox(world.registry, Vector2f{5.0F, 5.0F}, Vector2f{10.0F, 10.0F});
    world.registry.updateSystems(0.0F);
    world.registry.updateSystems(0.0F);
    cr_assert_eq(world.collisions.size(), 2);
}

Test(physics_collision_system, offset_moves_the_hitbox)
{
    World world;
    const Entity moved = createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});

    createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    world.registry.getComponent<Hitbox>(moved).offset = Vector2f{50.0F, 0.0F};
    world.registry.updateSystems(0.0F);
    cr_assert(world.collisions.empty());
}

Test(physics_collision_system, scale_resizes_the_hitbox)
{
    World world;
    const Entity scaled = createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});

    createBox(world.registry, Vector2f{20.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    world.registry.getComponent<Transform>(scaled).scale = Vector2f{3.0F, 3.0F};
    world.registry.updateSystems(0.0F);
    cr_assert_eq(world.collisions.size(), 1);
}

Test(physics_collision_system, entity_without_hitbox_is_ignored)
{
    World world;
    const Entity withoutHitbox = world.registry.createEntity();

    createBox(world.registry, Vector2f{0.0F, 0.0F}, Vector2f{10.0F, 10.0F});
    world.registry.addComponent(withoutHitbox, Transform{});
    world.registry.updateSystems(0.0F);
    cr_assert(world.collisions.empty());
}

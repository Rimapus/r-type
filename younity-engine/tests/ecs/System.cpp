/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/System
** File description:
** System unit tests
*/

#include <criterion/criterion.h>

#include <vector>

#include "younity/ecs/Registry.hpp"

using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::System;

namespace {

struct Position
{
    int x;
    int y;
};

struct Velocity
{
    int x;
    int y;
};

class MovementSystem : public System
{
public:
    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)dt;
        for (const Entity entity : entities) {
            Position &position = registry.getComponent<Position>(entity);
            const Velocity &velocity = registry.getComponent<Velocity>(entity);

            position.x += velocity.x;
            position.y += velocity.y;
        }
    }
};

// Keeps what it receives, to check the entities and the delta time given to a system
class RecordSystem : public System
{
public:
    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)registry;
        this->lastEntities = entities;
        this->lastDt = dt;
    }

    std::vector<Entity> lastEntities;
    float lastDt = 0.0F;
};

// Writes its number in a list when it runs, to check the update order
class OrderSystem : public System
{
public:
    OrderSystem(std::vector<int> &order, int number)
        : _order(order), _number(number) {}

    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)registry;
        (void)entities;
        (void)dt;
        this->_order.push_back(this->_number);
    }

private:
    std::vector<int> &_order;
    int _number;
};

class DestroySystem : public System
{
public:
    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)dt;
        for (const Entity entity : entities) {
            registry.destroyEntity(entity);
        }
    }
};

} // namespace

Test(ecs_system, gets_the_entities_with_its_components)
{
    Registry registry;
    RecordSystem &system = registry.addSystem<RecordSystem, Position, Velocity>();
    const Entity moving = registry.createEntity();
    const Entity still = registry.createEntity();

    registry.addComponent(moving, Position{0, 0});
    registry.addComponent(moving, Velocity{1, 1});
    registry.addComponent(still, Position{0, 0});
    registry.updateSystems(0.0F);
    cr_assert_eq(system.lastEntities.size(), 1);
    cr_assert(system.lastEntities[0] == moving);
}

Test(ecs_system, gets_the_entities_created_before_it)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Position{0, 0});
    registry.addComponent(entity, Velocity{1, 1});
    RecordSystem &system = registry.addSystem<RecordSystem, Position, Velocity>();

    registry.updateSystems(0.0F);
    cr_assert_eq(system.lastEntities.size(), 1);
}

Test(ecs_system, entity_leaves_when_a_component_is_removed)
{
    Registry registry;
    RecordSystem &system = registry.addSystem<RecordSystem, Position, Velocity>();
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Position{0, 0});
    registry.addComponent(entity, Velocity{1, 1});
    registry.removeComponent<Velocity>(entity);
    registry.updateSystems(0.0F);
    cr_assert(system.lastEntities.empty());
}

Test(ecs_system, entity_leaves_when_destroyed)
{
    Registry registry;
    RecordSystem &system = registry.addSystem<RecordSystem, Position, Velocity>();
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Position{0, 0});
    registry.addComponent(entity, Velocity{1, 1});
    registry.destroyEntity(entity);
    registry.updateSystems(0.0F);
    cr_assert(system.lastEntities.empty());
}

Test(ecs_system, update_runs_the_system)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.addSystem<MovementSystem, Position, Velocity>();
    registry.addComponent(entity, Position{0, 0});
    registry.addComponent(entity, Velocity{2, 3});
    registry.updateSystems(1.0F);
    cr_assert_eq(registry.getComponent<Position>(entity).x, 2);
    cr_assert_eq(registry.getComponent<Position>(entity).y, 3);
}

Test(ecs_system, systems_run_in_the_order_they_are_added)
{
    Registry registry;
    std::vector<int> order;

    registry.addSystem<OrderSystem>(order, 1);
    registry.addSystem<OrderSystem>(order, 2);
    registry.addSystem<OrderSystem>(order, 3);
    registry.updateSystems(1.0F);
    cr_assert(order == (std::vector<int>{1, 2, 3}));
}

Test(ecs_system, update_gives_the_delta_time)
{
    Registry registry;
    RecordSystem &system = registry.addSystem<RecordSystem>();

    registry.updateSystems(0.5F);
    cr_assert_float_eq(system.lastDt, 0.5F, 0.0001F);
}

Test(ecs_system, entity_destroyed_by_a_system_is_dead_after_the_update)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.addSystem<DestroySystem, Position>();
    registry.addComponent(entity, Position{0, 0});
    registry.updateSystems(1.0F);
    cr_assert_not(registry.isAlive(entity));
}

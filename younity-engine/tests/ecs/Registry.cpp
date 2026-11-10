/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/Registry
** File description:
** Registry unit tests
*/

#include <criterion/criterion.h>

#include <stdexcept>
#include <string_view>
#include <utility>
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

struct Health
{
    int value;
};

template <std::size_t Number>
struct OverflowTag
{
    int value = 0;
};

template <std::size_t... Numbers>
void addOverflowTags(Registry &registry, Entity entity, std::index_sequence<Numbers...>)
{
    (registry.addComponent(entity, OverflowTag<Numbers>{}), ...);
}

class CountingSystem : public System
{
public:
    explicit CountingSystem(int &count)
        : _count(count) {}

    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)registry;
        (void)entities;
        (void)dt;
        ++this->_count;
    }

private:
    int &_count;
};

class ClearSystem : public System
{
public:
    explicit ClearSystem(bool &caught)
        : _caught(caught) {}

    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)entities;
        (void)dt;
        try {
            registry.clear();
        } catch (const std::logic_error &) {
            this->_caught = true;
        }
    }

private:
    bool &_caught;
};

class EmptySystem : public System
{
public:
    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)registry;
        (void)entities;
        (void)dt;
    }
};

class AddingSystem : public System
{
public:
    explicit AddingSystem(bool &caught)
        : _caught(caught) {}

    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)entities;
        (void)dt;
        try {
            registry.addSystem<EmptySystem>();
        } catch (const std::logic_error &) {
            this->_caught = true;
        }
    }

private:
    bool &_caught;
};

class ThrowingSystem : public System
{
public:
    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)registry;
        (void)entities;
        (void)dt;
        throw std::runtime_error("the system failed");
    }
};

class DestroyThenThrowSystem : public System
{
public:
    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)dt;
        for (const Entity entity : entities) {
            registry.destroyEntity(entity);
        }
        throw std::runtime_error("the system failed after destroying its entities");
    }
};

} // namespace

Test(ecs_registry, created_entity_is_alive)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    cr_assert(registry.isAlive(entity));
}

Test(ecs_registry, created_entities_have_different_ids)
{
    Registry registry;
    const Entity first = registry.createEntity();
    const Entity second = registry.createEntity();

    cr_assert_neq(first.getId(), second.getId());
}

Test(ecs_registry, destroyed_entity_is_not_alive)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.destroyEntity(entity);
    cr_assert_not(registry.isAlive(entity));
}

Test(ecs_registry, destroyed_id_is_reused)
{
    Registry registry;
    const Entity destroyed = registry.createEntity();

    registry.destroyEntity(destroyed);
    const Entity reused = registry.createEntity();

    cr_assert_eq(reused.getId(), destroyed.getId());
}

Test(ecs_registry, entity_created_with_an_id_has_it)
{
    Registry registry;
    const Entity entity = registry.createEntity(2);

    cr_assert_eq(entity.getId(), 2U);
    cr_assert(registry.isAlive(entity));
    cr_assert_not(registry.isAlive(Entity(0)));
    cr_assert_not(registry.isAlive(Entity(1)));
    cr_assert_lt(registry.createEntity().getId(), 2U);
    cr_assert_lt(registry.createEntity().getId(), 2U);
    cr_assert_eq(registry.createEntity().getId(), 3U);
}

Test(ecs_registry, entity_created_with_a_free_id_takes_it)
{
    Registry registry;
    const Entity first = registry.createEntity();

    (void)registry.createEntity();
    registry.destroyEntity(first);
    cr_assert_eq(registry.createEntity(0).getId(), 0U);
    cr_assert_eq(registry.createEntity().getId(), 2U);
}

Test(ecs_registry, entity_created_with_an_alive_id_throws)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    cr_assert_throw((void)registry.createEntity(entity.getId()), std::invalid_argument);
}

Test(ecs_registry, add_then_get_a_component)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Position{1, 2});
    cr_assert(registry.hasComponent<Position>(entity));
    cr_assert_eq(registry.getComponent<Position>(entity).x, 1);
    cr_assert_eq(registry.getComponent<Position>(entity).y, 2);
}

Test(ecs_registry, remove_a_component)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Position{1, 2});
    registry.removeComponent<Position>(entity);
    cr_assert_not(registry.hasComponent<Position>(entity));
}

Test(ecs_registry, components_of_different_types)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Position{1, 2});
    registry.addComponent(entity, Health{100});
    registry.removeComponent<Health>(entity);
    cr_assert(registry.hasComponent<Position>(entity));
    cr_assert_not(registry.hasComponent<Health>(entity));
}

Test(ecs_registry, destroy_entity_removes_its_components)
{
    Registry registry;
    const Entity destroyed = registry.createEntity();

    registry.addComponent(destroyed, Position{1, 2});
    registry.destroyEntity(destroyed);
    const Entity reused = registry.createEntity();

    cr_assert_not(registry.hasComponent<Position>(reused));
}

Test(ecs_registry, get_a_missing_component_throws)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    cr_assert_not(registry.hasComponent<Position>(entity));
    cr_assert_throw((void)registry.getComponent<Position>(entity), std::out_of_range);
}

Test(ecs_registry, dead_entity_throws)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.destroyEntity(entity);
    cr_assert_throw(registry.addComponent(entity, Position{1, 2}), std::invalid_argument);
}

Test(ecs_registry, too_many_component_types_throws)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    cr_assert_throw(
        (addOverflowTags(registry, entity, std::make_index_sequence<65>())), std::length_error);
}

Test(ecs_registry, entities_with_returns_the_ones_having_all_the_components)
{
    Registry registry;
    const Entity both = registry.createEntity();
    const Entity onlyPosition = registry.createEntity();
    const Entity onlyHealth = registry.createEntity();

    registry.addComponent(both, Position{1, 2});
    registry.addComponent(both, Health{10});
    registry.addComponent(onlyPosition, Position{3, 4});
    registry.addComponent(onlyHealth, Health{5});
    const std::vector<Entity> found = registry.getEntitiesWith<Position, Health>();
    cr_assert_eq(found.size(), 1U);
    cr_assert_eq(found[0], both);
}

Test(ecs_registry, entities_with_are_sorted_by_id)
{
    Registry registry;
    const Entity first = registry.createEntity();
    const Entity second = registry.createEntity();

    // Added in the reverse order of the ids
    registry.addComponent(second, Position{0, 0});
    registry.addComponent(first, Position{0, 0});
    const std::vector<Entity> found = registry.getEntitiesWith<Position>();
    cr_assert_eq(found.size(), 2U);
    cr_assert_eq(found[0], first);
    cr_assert_eq(found[1], second);
}

Test(ecs_registry, entities_with_skips_destroyed_entities)
{
    Registry registry;
    const Entity kept = registry.createEntity();
    const Entity destroyed = registry.createEntity();

    registry.addComponent(kept, Position{0, 0});
    registry.addComponent(destroyed, Position{0, 0});
    registry.destroyEntity(destroyed);
    const std::vector<Entity> found = registry.getEntitiesWith<Position>();
    cr_assert_eq(found.size(), 1U);
    cr_assert_eq(found[0], kept);
}

Test(ecs_registry, entities_with_a_removed_component_are_not_found)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Position{0, 0});
    registry.removeComponent<Position>(entity);
    cr_assert(registry.getEntitiesWith<Position>().empty());
}

Test(ecs_registry, entities_with_a_type_never_added_is_empty)
{
    const Registry registry;

    // const: the query must not register the component type
    cr_assert(registry.getEntitiesWith<Position>().empty());
}

Test(ecs_registry, entities_are_the_alive_ones_sorted_by_id)
{
    Registry registry;
    const Entity first = registry.createEntity();
    const Entity destroyed = registry.createEntity();
    const Entity last = registry.createEntity();

    registry.destroyEntity(destroyed);
    cr_assert(registry.getEntities() == (std::vector<Entity>{first, last}));
}

Test(ecs_registry, entities_include_a_reused_id)
{
    Registry registry;
    const Entity kept = registry.createEntity();

    registry.destroyEntity(registry.createEntity());
    const Entity reused = registry.createEntity();
    cr_assert(registry.getEntities() == (std::vector<Entity>{kept, reused}));
}

Test(ecs_registry, entities_of_an_empty_registry_is_empty)
{
    const Registry registry;

    cr_assert(registry.getEntities().empty());
}

Test(ecs_registry, clear_removes_the_entities_components_and_systems)
{
    Registry registry;
    const Entity entity = registry.createEntity();
    int updates = 0;

    registry.addComponent(entity, Position{1, 2});
    registry.addSystem<CountingSystem>(updates);
    registry.updateSystems(0.0F);
    cr_assert_eq(updates, 1);
    registry.clear();
    cr_assert(registry.getEntities().empty());
    cr_assert_not(registry.isAlive(entity));
    registry.updateSystems(0.0F);
    cr_assert_eq(updates, 1);
    const Entity recreated = registry.createEntity();

    cr_assert_eq(recreated.getId(), 0U);
    cr_assert_not(registry.hasComponent<Position>(recreated));
}

Test(ecs_registry, destroy_entities_keeps_the_systems)
{
    Registry registry;
    const Entity entity = registry.createEntity();
    int updates = 0;

    registry.addComponent(entity, Position{1, 2});
    registry.addSystem<CountingSystem>(updates);
    registry.destroyEntities();
    cr_assert(registry.getEntities().empty());
    cr_assert_not(registry.isAlive(entity));
    registry.updateSystems(0.0F);
    cr_assert_eq(updates, 1);
    const Entity recreated = registry.createEntity();

    cr_assert_eq(recreated.getId(), 0U);
    cr_assert_not(registry.hasComponent<Position>(recreated));
}

Test(ecs_registry, clear_resets_the_component_type_numbers)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    addOverflowTags(registry, entity, std::make_index_sequence<64>());
    cr_assert_throw(registry.addComponent(entity, OverflowTag<64>{}), std::length_error);
    registry.clear();
    const Entity recreated = registry.createEntity();

    cr_assert_eq(recreated.getId(), 0U);
    registry.addComponent(recreated, OverflowTag<64>{});
    cr_assert(registry.hasComponent<OverflowTag<64>>(recreated));
}

Test(ecs_registry, clear_during_update_throws)
{
    Registry registry;
    bool caught = false;

    registry.addSystem<ClearSystem>(caught);
    registry.updateSystems(0.0F);
    cr_assert(caught);
}

Test(ecs_registry, add_system_during_update_throws)
{
    Registry registry;
    bool caught = false;

    registry.addSystem<AddingSystem>(caught);
    registry.updateSystems(0.0F);
    cr_assert(caught);
}

Test(ecs_registry, update_systems_clears_the_update_state_after_a_throwing_system)
{
    Registry registry;

    registry.addSystem<ThrowingSystem>();
    cr_assert_throw(registry.updateSystems(0.0F), std::runtime_error);
    const Entity entity = registry.createEntity();

    registry.destroyEntity(entity);
    cr_assert_not(registry.isAlive(entity));
    registry.clear();
}

Test(ecs_registry, update_systems_destroys_pending_entities_when_a_system_throws)
{
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Position{1, 2});
    registry.addSystem<DestroyThenThrowSystem, Position>();
    cr_assert_throw(registry.updateSystems(0.0F), std::runtime_error);
    cr_assert_not(registry.isAlive(entity));
}

Test(ecs_registry, update_systems_clears_the_update_state_after_the_profiler_throws)
{
    Registry registry;
    int updates = 0;

    registry.addSystem<CountingSystem>(updates);
    registry.setSystemProfiler([](std::string_view name, double seconds) {
        (void)name;
        (void)seconds;
        throw std::runtime_error("the profiler failed");
    });
    cr_assert_throw(registry.updateSystems(0.0F), std::runtime_error);
    cr_assert_eq(updates, 1);
    const Entity entity = registry.createEntity();

    registry.destroyEntity(entity);
    cr_assert_not(registry.isAlive(entity));
    registry.clear();
}

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/ComponentRegistry
** File description:
** ComponentRegistry unit tests
*/

#include <criterion/criterion.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"

using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::Registry;

namespace {

struct Health
{
    int value = 100;
};

struct Score
{
    int value = 0;
};

} // namespace

Test(ecs_component_registry, get_a_component_by_name)
{
    ComponentRegistry components;

    components.add<Health>("Health").field("value", &Health::value);
    cr_assert_eq(components.get("Health").getName(), "Health");
    cr_assert(components.get("Health").getFieldNames() == (std::vector<std::string>{"value"}));
}

Test(ecs_component_registry, names_are_sorted)
{
    ComponentRegistry components;

    components.add<Score>("Score");
    components.add<Health>("Health");
    cr_assert(components.getNames() == (std::vector<std::string>{"Health", "Score"}));
}

Test(ecs_component_registry, unknown_name_throws)
{
    const ComponentRegistry components;

    cr_assert_throw((void)components.get("Health"), std::out_of_range);
}

Test(ecs_component_registry, name_used_twice_throws)
{
    ComponentRegistry components;

    components.add<Health>("Health");
    cr_assert_throw(components.add<Score>("Health"), std::invalid_argument);
}

Test(ecs_component_registry, type_registered_twice_throws)
{
    ComponentRegistry components;

    components.add<Health>("Health");
    cr_assert_throw(components.add<Health>("Life"), std::invalid_argument);
}

Test(ecs_component_registry, names_of_an_entity_are_its_registered_components)
{
    ComponentRegistry components;
    Registry registry;
    const Entity entity = registry.createEntity();

    components.add<Score>("Score");
    components.add<Health>("Health");
    registry.addComponent(entity, Score{});
    registry.addComponent(entity, Health{});
    cr_assert(components.getNamesOf(registry, entity) == (std::vector<std::string>{"Health", "Score"}));
}

Test(ecs_component_registry, names_of_an_entity_skip_unregistered_components)
{
    ComponentRegistry components;
    Registry registry;
    const Entity entity = registry.createEntity();

    components.add<Health>("Health");
    registry.addComponent(entity, Score{});
    cr_assert(components.getNamesOf(registry, entity).empty());
}

Test(ecs_component_registry, names_of_a_dead_entity_throws)
{
    ComponentRegistry components;
    Registry registry;
    const Entity entity = registry.createEntity();

    registry.destroyEntity(entity);
    cr_assert_throw((void)components.getNamesOf(registry, entity), std::invalid_argument);
}

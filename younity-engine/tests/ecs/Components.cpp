/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/Components
** File description:
** Built-in components unit tests
*/

#include <criterion/criterion.h>

#include <string>
#include <vector>

#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Components.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"

using younity::Vector2f;
using younity::ecs::ComponentRegistry;
using younity::ecs::Transform;
using younity::ecs::Velocity;

Test(ecs_components, transform_default_values)
{
    const Transform transform;

    cr_assert(transform.position == (Vector2f{0.0F, 0.0F}));
    cr_assert_float_eq(transform.rotation, 0.0F, 0.0001F);
    cr_assert(transform.scale == (Vector2f{1.0F, 1.0F}));
}

Test(ecs_components, velocity_default_is_zero)
{
    const Velocity velocity;

    cr_assert(velocity.value == (Vector2f{0.0F, 0.0F}));
}

Test(ecs_components, register_components_with_all_their_fields)
{
    ComponentRegistry components;

    younity::ecs::registerComponents(components);
    cr_assert(components.getNames() == (std::vector<std::string>{"Transform", "Velocity"}));
    cr_assert(components.get("Transform").getFieldNames() == (std::vector<std::string>{"position", "rotation", "scale"}));
    cr_assert(components.get("Velocity").getFieldNames() == (std::vector<std::string>{"value"}));
}

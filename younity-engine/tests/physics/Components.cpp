/*
** EPITECH PROJECT, 2026
** younity-engine/tests/physics/Components
** File description:
** Physics components registration unit tests
*/

#include <criterion/criterion.h>

#include <string>
#include <vector>

#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/physics/Components.hpp"

using younity::ecs::ComponentRegistry;

Test(physics_components, register_components_with_all_their_fields)
{
    ComponentRegistry components;

    younity::physics::registerComponents(components);
    cr_assert(components.getNames() == (std::vector<std::string>{"Body", "Gravity", "Hitbox", "Solid"}));
    cr_assert(components.get("Body").getFieldNames() == (std::vector<std::string>{"bounce", "onGround"}));
    cr_assert(components.get("Gravity").getFieldNames() == (std::vector<std::string>{"scale"}));
    cr_assert(components.get("Hitbox").getFieldNames() == (std::vector<std::string>{"size", "offset"}));
    cr_assert(components.get("Solid").getFieldNames().empty());
}

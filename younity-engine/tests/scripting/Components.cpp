/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/Components
** File description:
** Components unit tests
*/

#include <criterion/criterion.h>

#include <string>
#include <vector>

#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/scripting/Components.hpp"

using younity::ecs::ComponentRegistry;

Test(scripting_components, register_the_script_with_its_path)
{
    ComponentRegistry components;

    younity::scripting::registerComponents(components);
    cr_assert(components.getNames() == (std::vector<std::string>{"Script"}));
    cr_assert(components.get("Script").getFieldNames() == (std::vector<std::string>{"path"}));
}

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/Components
** File description:
** Components source file
*/

#include "younity/scripting/Components.hpp"
#include "younity/scripting/components/Script.hpp"

namespace younity::scripting {

void registerComponents(ecs::ComponentRegistry &components)
{
    // The instance is not a field: a loaded scene starts its scripts again
    components.add<Script>("Script").field("path", &Script::path);
}

} // namespace younity::scripting

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/EcsLibrary
** File description:
** EcsLibrary header file, private to the scripting module
*/

#pragma once

#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include <sol/sol.hpp>
#include <unordered_set>

namespace younity::scripting {

/**
 * @brief What the ecs table of the scripts works on.
 */
struct EcsContext
{
    // The component types a script reaches by their name
    const ecs::ComponentRegistry &components;
    // The registry of the step running, null outside a step
    ecs::Registry *registry = nullptr;
    // The entities a script destroyed during the step: they are destroyed at its end, but their scripts stop now
    std::unordered_set<ecs::Entity::Id> destroyed;
};

/**
 * @brief Add the ecs table to the interpreter: the functions a script uses to create and destroy entities,
 *        and to read and write their components by name.
 *
 * @param lua The interpreter
 * @param context The registries the functions work on, it must outlive the interpreter
 */
void openEcsLibrary(sol::state &lua, EcsContext &context);

} // namespace younity::scripting

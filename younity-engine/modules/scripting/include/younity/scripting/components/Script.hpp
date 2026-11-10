/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/include/younity/scripting/components/Script
** File description:
** Script header file
*/

#pragma once

#include <cstdint>
#include <string>

namespace younity::scripting {

/**
 * @brief Gives an entity the behaviour written in a Lua file, run by the ScriptSystem.
 */
struct Script
{
    // The Lua file, relative to the folder of the ScriptSystem, e.g. "scripts/spin.lua"
    std::string path;
    // Set by the ScriptSystem when the script starts, 0 before: tells a new entity that reuses an id from the old one
    std::uint64_t instance{0};
};

} // namespace younity::scripting

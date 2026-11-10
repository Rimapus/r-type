/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/include/younity/scripting/Components
** File description:
** Components header file
*/

#pragma once

#include "younity/ecs/ComponentRegistry.hpp"

namespace younity::scripting {

/**
 * @brief Register the component of the scripting module, Script, with its path.
 *
 * @param components The component registry of the game
 *
 * @throw std::invalid_argument if Script is already registered.
 */
void registerComponents(ecs::ComponentRegistry &components);

} // namespace younity::scripting

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/Components
** File description:
** Components header file
*/

#pragma once

#include "younity/ecs/ComponentRegistry.hpp"

namespace younity::physics {

/**
 * @brief Register the components of the physics module, Body, Gravity, Hitbox and Solid, with all their fields.
 *
 * @param components The component registry of the game
 *
 * @throw std::invalid_argument if one of these components is already registered.
 */
void registerComponents(ecs::ComponentRegistry &components);

} // namespace younity::physics

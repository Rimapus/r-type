/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/Components
** File description:
** Components header file
*/

#pragma once

#include "younity/ecs/ComponentRegistry.hpp"

namespace younity::ecs {

/**
 * @brief Register the components of the ecs module, Transform and Velocity, with all their fields.
 *
 * @param components The component registry of the game
 *
 * @throw std::invalid_argument if one of these components is already registered.
 */
void registerComponents(ecs::ComponentRegistry &components);

} // namespace younity::ecs

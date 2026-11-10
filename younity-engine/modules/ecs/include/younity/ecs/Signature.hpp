/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/Signature
** File description:
** Signature header file
*/

#pragma once

#include <bitset>
#include <cstddef>

namespace younity::ecs {

/**
 * @brief Maximum number of component types in a registry.
 */
constexpr std::size_t MAX_COMPONENTS = 64;

/**
 * @brief Set of component types: the bit n is set if the component type number n is in the set.
 */
using Signature = std::bitset<MAX_COMPONENTS>;

} // namespace younity::ecs

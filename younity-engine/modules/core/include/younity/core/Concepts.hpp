/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/Concepts
** File description:
** Concepts header file
*/

#pragma once

#include <concepts>

namespace younity {

/**
 * @brief A number type: an integer or a floating point number.
 */
template <typename T>
concept Numeric = (std::integral<T> || std::floating_point<T>) && !std::same_as<T, bool>;

} // namespace younity

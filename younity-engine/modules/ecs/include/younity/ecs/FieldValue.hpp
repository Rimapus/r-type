/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/FieldValue
** File description:
** FieldValue header file
*/

#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "younity/core/Color.hpp"
#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"

namespace younity::ecs {

/**
 * @brief The type of a component field, one of the types of FieldValue.
 */
enum class FieldType {
    /** @brief A bool. */
    Bool,
    /** @brief An integer of any size, given as an int. */
    Int,
    /** @brief A float. */
    Float,
    /** @brief A std::string. */
    String,
    /** @brief A Vector2f. */
    Vector2f,
    /** @brief A Color. */
    Color,
    /** @brief A std::optional<Recti>. */
    OptionalRecti,
    /** @brief A std::vector<Recti>, e.g. the frames of an animation. */
    RectiVector,
};

/**
 * @brief The value of a component field read or written by its name, e.g. by an editor, a scene file or a script.
 *        An integer field of any size is given as an int.
 */
using FieldValue =
    std::variant<bool, int, float, std::string, Vector2f, Color, std::optional<Recti>, std::vector<Recti>>;

} // namespace younity::ecs

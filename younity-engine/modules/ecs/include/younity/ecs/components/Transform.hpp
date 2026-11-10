/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/components/Transform
** File description:
** Transform header file
*/

#pragma once

#include "younity/core/Vector2.hpp"

namespace younity::ecs {

/**
 * @brief Where an entity is, how it is rotated and how big it is.
 */
struct Transform
{
    // In pixels
    Vector2f position{};
    // In degrees
    float rotation = 0.0F;
    // 1 is the original size
    Vector2f scale{1.0F, 1.0F};
};

} // namespace younity::ecs

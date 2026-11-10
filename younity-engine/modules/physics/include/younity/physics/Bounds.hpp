/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/include/younity/physics/Bounds
** File description:
** Bounds header file
*/

#pragma once

#include "younity/core/Rect.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/physics/components/Hitbox.hpp"

namespace younity::physics {

/**
 * @brief Get the rectangle of a hitbox in the world.
 *        The scale of the Transform is applied to the hitbox, its rotation is ignored.
 *
 * @param transform The Transform of the entity owning the hitbox
 * @param hitbox The hitbox of the entity
 *
 * @return The rectangle of the hitbox, placed from the position of the Transform.
 */
[[nodiscard]] Rectf getBounds(const ecs::Transform &transform, const Hitbox &hitbox);

} // namespace younity::physics

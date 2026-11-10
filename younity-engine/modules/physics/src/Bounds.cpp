/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/src/Bounds
** File description:
** Bounds source file
*/

#include "younity/physics/Bounds.hpp"

namespace younity::physics {

Rectf getBounds(const ecs::Transform &transform, const Hitbox &hitbox)
{
    Rectf bounds;

    bounds.left = transform.position.x + hitbox.offset.x * transform.scale.x;
    bounds.top = transform.position.y + hitbox.offset.y * transform.scale.y;
    bounds.width = hitbox.size.x * transform.scale.x;
    bounds.height = hitbox.size.y * transform.scale.y;
    return bounds;
}

} // namespace younity::physics

/*
** EPITECH PROJECT, 2026
** j-type/common/src/Targeting
** File description:
** Targeting source file
*/

#include "jtype/common/Targeting.hpp"

#include <limits>

#include "jtype/common/components/Player.hpp"
#include "younity/ecs/components/Transform.hpp"

namespace jtype {

std::optional<younity::Vector2f> findNearestPlayer(const younity::ecs::Registry &registry, younity::Vector2f from)
{
    std::optional<younity::Vector2f> nearest;
    float shortest = std::numeric_limits<float>::max();

    for (const younity::ecs::Entity player : registry.getEntitiesWith<Player, younity::ecs::Transform>()) {
        const younity::Vector2f position = registry.getComponent<younity::ecs::Transform>(player).position;
        const younity::Vector2f offset = position - from;
        const float distance = (offset.x * offset.x) + (offset.y * offset.y);

        if (distance < shortest) {
            shortest = distance;
            nearest = position;
        }
    }
    return nearest;
}

} // namespace jtype

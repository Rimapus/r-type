/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/src/Components
** File description:
** Components source file
*/

#include "younity/ecs/Components.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"

namespace younity::ecs {

void registerComponents(ecs::ComponentRegistry &components)
{
    components.add<Transform>("Transform")
        .field("position", &Transform::position)
        .field("rotation", &Transform::rotation)
        .field("scale", &Transform::scale);
    components.add<Velocity>("Velocity").field("value", &Velocity::value);
}

} // namespace younity::ecs

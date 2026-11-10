/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/src/Components
** File description:
** Components source file
*/

#include "younity/physics/Components.hpp"
#include "younity/physics/components/Body.hpp"
#include "younity/physics/components/Gravity.hpp"
#include "younity/physics/components/Hitbox.hpp"
#include "younity/physics/components/Solid.hpp"

namespace younity::physics {

void registerComponents(ecs::ComponentRegistry &components)
{
    components.add<Body>("Body").field("bounce", &Body::bounce).field("onGround", &Body::onGround);
    components.add<Gravity>("Gravity").field("scale", &Gravity::scale);
    components.add<Hitbox>("Hitbox").field("size", &Hitbox::size).field("offset", &Hitbox::offset);
    components.add<Solid>("Solid");
}

} // namespace younity::physics

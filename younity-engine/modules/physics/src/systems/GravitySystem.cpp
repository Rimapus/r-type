/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/src/systems/GravitySystem
** File description:
** GravitySystem source file
*/

#include "younity/physics/systems/GravitySystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/physics/components/Gravity.hpp"

namespace younity::physics {

GravitySystem::GravitySystem(Vector2f gravity)
    : _gravity(gravity) {}

void GravitySystem::update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt)
{
    for (const ecs::Entity entity : entities) {
        const Gravity &gravity = registry.getComponent<Gravity>(entity);
        ecs::Velocity &velocity = registry.getComponent<ecs::Velocity>(entity);

        velocity.value += this->_gravity * gravity.scale * dt;
    }
}

} // namespace younity::physics

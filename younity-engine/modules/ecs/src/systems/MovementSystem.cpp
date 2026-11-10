/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/src/systems/MovementSystem
** File description:
** MovementSystem source file
*/

#include "younity/ecs/systems/MovementSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"

namespace younity::ecs {

void MovementSystem::update(Registry &registry, const std::vector<Entity> &entities, float dt)
{
    for (const Entity entity : entities) {
        const Velocity &velocity = registry.getComponent<Velocity>(entity);
        Transform &transform = registry.getComponent<Transform>(entity);

        transform.position += velocity.value * dt;
    }
}

} // namespace younity::ecs

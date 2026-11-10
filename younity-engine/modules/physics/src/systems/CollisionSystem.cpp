/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/src/systems/CollisionSystem
** File description:
** CollisionSystem source file
*/

#include "younity/physics/systems/CollisionSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/physics/Bounds.hpp"
#include "younity/physics/CollisionEvent.hpp"
#include "younity/physics/components/Hitbox.hpp"
#include <cstddef>

namespace younity::physics {

CollisionSystem::CollisionSystem(EventBus &bus)
    : _bus(bus) {}

void CollisionSystem::update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float /* dt */)
{
    std::vector<Rectf> bounds;

    bounds.reserve(entities.size());
    for (const ecs::Entity entity : entities) {
        const ecs::Transform &transform = registry.getComponent<ecs::Transform>(entity);
        const Hitbox &hitbox = registry.getComponent<Hitbox>(entity);

        bounds.push_back(getBounds(transform, hitbox));
    }

    for (std::size_t first = 0; first < entities.size(); ++first) {
        for (std::size_t second = first + 1; second < entities.size(); ++second) {
            if (bounds[first].intersects(bounds[second])) {
                this->_bus.publish(CollisionEvent{entities[first], entities[second]});
            }
        }
    }
}

} // namespace younity::physics

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/physics/src/systems/SolidCollisionSystem
** File description:
** SolidCollisionSystem source file
*/

#include "younity/physics/systems/SolidCollisionSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/physics/Bounds.hpp"
#include "younity/physics/components/Body.hpp"
#include "younity/physics/components/Hitbox.hpp"
#include "younity/physics/components/Solid.hpp"
#include <algorithm>

namespace younity::physics {

namespace {

/**
 * @brief Push a body out of a solid along one axis, and send back the speed going into the solid.
 *
 * @param position The position of the body on this axis
 * @param speed The velocity of the body on this axis
 * @param push The signed distance moving the body out of the solid
 * @param bounce The bounce factor of the body
 */
void pushOut(float &position, float &speed, float push, float bounce)
{
    position += push;
    if (speed * push < 0.0F) {
        speed = -speed * bounce;
    }
}

} // namespace

void SolidCollisionSystem::update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float /* dt */)
{
    // A system receives one list of entities: the solids are a second one, asked to the registry
    const std::vector<ecs::Entity> solids = registry.getEntitiesWith<ecs::Transform, Hitbox, Solid>();

    for (const ecs::Entity entity : entities) {
        ecs::Transform &transform = registry.getComponent<ecs::Transform>(entity);
        ecs::Velocity &velocity = registry.getComponent<ecs::Velocity>(entity);
        const Hitbox &hitbox = registry.getComponent<Hitbox>(entity);
        Body &body = registry.getComponent<Body>(entity);

        body.onGround = false;
        for (const ecs::Entity solid : solids) {
            // A body that is also a solid would push itself away
            if (solid == entity) {
                continue;
            }
            const Rectf bounds = getBounds(transform, hitbox);
            const Rectf obstacle = getBounds(registry.getComponent<ecs::Transform>(solid), registry.getComponent<Hitbox>(solid));

            if (!bounds.intersects(obstacle)) {
                continue;
            }
            const float overlapX = std::min(bounds.left + bounds.width, obstacle.left + obstacle.width) - std::max(bounds.left, obstacle.left);
            const float overlapY = std::min(bounds.top + bounds.height, obstacle.top + obstacle.height) - std::max(bounds.top, obstacle.top);
            const bool isLeft = bounds.left + bounds.width / 2.0F < obstacle.left + obstacle.width / 2.0F;
            const bool isAbove = bounds.top + bounds.height / 2.0F < obstacle.top + obstacle.height / 2.0F;

            if (overlapX < overlapY) {
                pushOut(transform.position.x, velocity.value.x, isLeft ? -overlapX : overlapX, body.bounce);
            } else {
                pushOut(transform.position.y, velocity.value.y, isAbove ? -overlapY : overlapY, body.bounce);
                body.onGround = body.onGround || isAbove;
            }
        }
    }
}

} // namespace younity::physics

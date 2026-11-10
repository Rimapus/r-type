/*
** EPITECH PROJECT, 2026
** j-type/common/src/systems/BoundsSystem
** File description:
** BoundsSystem source file
*/

#include "jtype/common/systems/BoundsSystem.hpp"

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Obstacle.hpp"
#include "jtype/common/components/PowerUp.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"

namespace jtype {

void BoundsSystem::update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float /* dt */)
{
    for (const younity::ecs::Entity entity : entities) {
        younity::Vector2f &position = registry.getComponent<younity::ecs::Transform>(entity).position;

        // Past an edge by the length of the longest missile: the missile cannot be seen anymore
        const bool missileLeft = position.x < -MISSILE_SIZE.x || position.x > WORLD_WIDTH + MISSILE_SIZE.x ||
                                 position.y < -MISSILE_SIZE.x || position.y > WORLD_HEIGHT + MISSILE_SIZE.x;

        if (registry.hasComponent<Missile>(entity) && missileLeft) {
            registry.destroyEntity(entity);
        } else if (registry.hasComponent<Enemy>(entity) && position.x - BYDO_ORIGIN.x + BYDO_SIZE.x < 0.0F) {
            registry.destroyEntity(entity);
        } else if (registry.hasComponent<PowerUp>(entity) && position.x + (POWER_UP_SIZE.x / 2.0F) < 0.0F) {
            registry.destroyEntity(entity);
        } else if (registry.hasComponent<Obstacle>(entity) && registry.getComponent<Obstacle>(entity).kind != ObstacleKind::Terrain &&
                   position.x + (OBSTACLE_SIZE.x / 2.0F) < 0.0F) {
            registry.destroyEntity(entity);
        }
    }
}

} // namespace jtype

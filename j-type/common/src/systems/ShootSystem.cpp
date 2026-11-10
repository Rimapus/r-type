/*
** EPITECH PROJECT, 2026
** j-type/common/src/systems/ShootSystem
** File description:
** ShootSystem source file
*/

#include "jtype/common/systems/ShootSystem.hpp"

#include <cmath>

#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Input.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Player.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"

namespace jtype {

void ShootSystem::update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt)
{
    for (const younity::ecs::Entity entity : entities) {
        Player &player = registry.getComponent<Player>(entity);

        player.shootCooldown -= dt;
        if ((player.inputs & input::SHOOT) == 0 || player.shootCooldown > 0.0F) {
            continue;
        }
        player.shootCooldown = SHOOT_COOLDOWN;

        const younity::Vector2f muzzle = registry.getComponent<younity::ecs::Transform>(entity).position + MUZZLE_OFFSET;

        if (player.weapon == Weapon::Spread) {
            for (const float angle : {-SPREAD_ANGLE, 0.0F, SPREAD_ANGLE}) {
                spawnPellet(registry, muzzle, younity::Vector2f{std::cos(angle), std::sin(angle)}, player.slot);
            }
        } else {
            spawnMissile(registry, muzzle, player.slot);
        }
    }
}

} // namespace jtype

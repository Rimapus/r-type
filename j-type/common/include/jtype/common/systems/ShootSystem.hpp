/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/systems/ShootSystem
** File description:
** ShootSystem header file
*/

#pragma once

#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace jtype {

/**
 * @brief Fires when a player holds the shoot input, once per SHOOT_COOLDOWN: a missile with the plasma, three pellets
 *        SPREAD_ANGLE apart with the spread shot.
 *        Add it with registry.addSystem<ShootSystem, Player, Transform>().
 */
class ShootSystem : public younity::ecs::System
{
public:
    void update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt) override;
};

} // namespace jtype

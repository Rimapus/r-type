/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Combat
** File description:
** Combat header file
*/

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "jtype/common/PowerUpType.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/physics/CollisionEvent.hpp"

namespace jtype {

/**
 * @brief An entity the combat destroyed, and who destroyed it.
 */
struct Death
{
    younity::ecs::Entity entity;
    // The slot of the player whose shot destroyed it, none for a crash or a Bydo missile
    std::optional<std::uint8_t> killer;
};

/**
 * @brief A power-up a ship took, and the player of the ship.
 */
struct PickUp
{
    std::uint8_t slot;
    PowerUpType type;
};

/**
 * @brief Take health points from a ship, a Bydo, a block of a wall or the eye of the boss. At 0, it explodes. A Bydo is
 *        destroyed, and the player who shot it earns its getKillPoints() and the power-up it carried. A block breaks. The
 *        eye takes the whole boss with it, leaves its wreck, and gives BOSS_POINTS. A ship loses a life: it stays where
 *        it was with full health, the plasma and RESPAWN_INVULNERABILITY, or leaves the game when that was its last
 *        life. A ship that is invulnerable is not hurt, and a shielded one loses its shield instead.
 *
 * @param registry The registry of the game
 * @param entity The ship, the Bydo, the block or the eye, having a Health and a Transform
 * @param damage The health points to take
 * @param shooter The slot of the player whose missile hit, none when it is not a missile of a player
 * @param deaths Where the destroyed entities are reported, none to ignore them
 */
void hurt(younity::ecs::Registry &registry, younity::ecs::Entity entity, int damage,
          std::optional<std::uint8_t> shooter = std::nullopt, std::vector<Death> *deaths = nullptr);

/**
 * @brief Apply what two touching entities do to each other, on the server:
 *        a missile of a player hurts a Bydo or the eye of the boss and is stopped by its body, a missile of a Bydo or of
 *        the boss hurts a ship, a Bydo hurts the ship it touches and is destroyed, the boss and the obstacles hurt the
 *        ship they touch and stay, every obstacle stops the missiles and a block breaks under the ones of the players,
 *        and a ship takes the power-up it touches: an extra life up to MAX_LIVES, a shield worth
 *        SHIELDED_HEALTH, or the spread shot. A missile hurts one target only.
 *
 * @param registry The registry of the game
 * @param collision The two touching entities, in any order
 * @param deaths Where the destroyed entities are reported, none to ignore them
 * @param pickUps Where the power-ups taken are reported, none to ignore them
 */
void resolveCollision(younity::ecs::Registry &registry, const younity::physics::CollisionEvent &collision,
                      std::vector<Death> *deaths = nullptr, std::vector<PickUp> *pickUps = nullptr);

} // namespace jtype

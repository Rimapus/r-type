/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/Missile
** File description:
** Missile header file
*/

#pragma once

#include <cstdint>
#include <optional>

namespace jtype {

/**
 * @brief How a missile looks: the plasma of a player or a pellet of its spread shot, the orb of a Bydo, or an orb or a
 *        spit of the boss.
 */
enum class MissileKind : std::uint8_t {
    Orb,
    BossOrb,
    BossSpit,
    Pellet,
};

/**
 * @brief A shot: a missile of a player hurts the Bydos and the eye of the boss, a missile of a Bydo or of the boss hurts
 *        the players.
 */
struct Missile
{
    // The slot of the player who shot it, to give the score to the right player; none for a Bydo or the boss
    std::optional<std::uint8_t> shooter;
    // The health points it takes from what it hits
    int damage = 1;
    // The sprite it is drawn with: a missile of a player is a plasma unless it is a Pellet
    MissileKind kind = MissileKind::Orb;
};

} // namespace jtype

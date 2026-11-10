/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/EntityType
** File description:
** EntityType header file
*/

#pragma once

#include <cstdint>
#include <optional>

namespace jtype {

/**
 * @brief What an entity of a snapshot is, so the client knows which sprite to draw.
 *        The players and their missiles have one type per slot, to give each player its own color.
 *        A value is a byte of the snapshots: a new one is added at the end, so the others keep their meaning.
 */
enum class EntityType : std::uint8_t {
    Player1 = 0,
    Player2 = 1,
    Player3 = 2,
    Player4 = 3,
    // The missiles of the players, in the order of their slots
    Missile1 = 4,
    Missile2 = 5,
    Missile3 = 6,
    Missile4 = 7,
    // A missile of a Bydo
    EnemyMissile = 8,
    Blobling = 9,
    Jelly = 10,
    SentryDrone = 11,
    // A ship or a Bydo that was just destroyed
    Explosion = 12,
    // The boss: its armored body, its eye, the eye once it lost half of its health, its two shots and its wreck
    IncubatorBody = 13,
    IncubatorEye = 14,
    IncubatorEyeAngry = 15,
    IncubatorOrb = 16,
    IncubatorSpit = 17,
    IncubatorWreck = 18,
    // The pellets of the spread shot of the players, in the order of their slots
    SpreadPellet1 = 19,
    SpreadPellet2 = 20,
    SpreadPellet3 = 21,
    SpreadPellet4 = 22,
    // The power-ups, in the order of PowerUpType
    ExtraLifePowerUp = 23,
    ShieldPowerUp = 24,
    SpreadShotPowerUp = 25,
    // A block of a wall, and the same block once it took a hit
    Block = 26,
    CrackedBlock = 27,
    // An end of a zapper, and a piece of its beam
    ZapperNode = 28,
    ZapperBeam = 29
};

/**
 * @brief Find the type of a missile shot by a player.
 *
 * @param slot The slot of the player, below MAX_PLAYERS
 *
 * @return Missile1 to Missile4.
 */
[[nodiscard]] EntityType getMissileType(std::uint8_t slot);

/**
 * @brief Find the type of a pellet of the spread shot of a player.
 *
 * @param slot The slot of the player, below MAX_PLAYERS
 *
 * @return SpreadPellet1 to SpreadPellet4.
 */
[[nodiscard]] EntityType getPelletType(std::uint8_t slot);

/**
 * @brief Find the player of a ship type.
 *
 * @param type The type of an entity
 *
 * @return The slot of the player for Player1 to Player4, nothing for another type.
 */
[[nodiscard]] std::optional<std::uint8_t> getPlayerSlot(EntityType type);

} // namespace jtype

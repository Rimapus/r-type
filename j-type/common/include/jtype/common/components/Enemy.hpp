/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/Enemy
** File description:
** Enemy header file
*/

#pragma once

#include <cstdint>
#include <optional>

#include "jtype/common/PowerUpType.hpp"

namespace jtype {

/**
 * @brief The kind of a Bydo, drawn with the sprite of the same name.
 */
enum class EnemyType : std::uint8_t {
    Blobling,
    Jelly,
    SentryDrone,
};

/**
 * @brief How a Bydo moves.
 */
enum class MovementPattern : std::uint8_t {
    // A straight line to the left
    Straight,
    // To the left, waving up and down
    SineWave,
    // Towards the nearest player for CHASE_DURATION, then to the left
    Chase,
};

/**
 * @brief Marks a Bydo: destroyed by the missiles of the players, it hurts the players it touches.
 */
struct Enemy
{
    EnemyType type = EnemyType::Blobling;
    MovementPattern pattern = MovementPattern::Straight;
    // The time left before the next shot, in seconds
    float shootCooldown = 0.0F;
    // The time since it appeared, in seconds, for the sine wave and the end of a chase
    float age = 0.0F;
    // The power-up it leaves when a player destroys it, e.g. the last Bydo of a group marked drop in the waves
    std::optional<PowerUpType> drop = std::nullopt;
};

} // namespace jtype

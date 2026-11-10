/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/Boss
** File description:
** Boss header file
*/

#pragma once

#include <cstdint>

namespace jtype {

/**
 * @brief The kind of a boss, drawn with the sprites of the same name.
 */
enum class BossType : std::uint8_t {
    Incubator,
};

/**
 * @brief A part of the boss: the body blocks the missiles of the players and hurts the ships it touches, the eye is the
 *        only part they can hurt.
 */
enum class BossPart : std::uint8_t {
    Body,
    Eye,
};

/**
 * @brief Marks a part of the boss, moved and fired by the BossSystem. There is one boss at a time.
 */
struct Boss
{
    BossPart part = BossPart::Body;
    // The time since the body reached the right edge, in seconds, for its rise and fall
    float age = 0.0F;
    // The time left before the next aimed orb and before the next fan of spits, in seconds, counted once at the edge
    float orbCooldown = 0.0F;
    float spitCooldown = 0.0F;
};

/**
 * @brief Marks the wreck of the boss, shown to the players for BOSS_WRECK_DURATION once its eye is destroyed.
 */
struct BossWreck
{
};

} // namespace jtype

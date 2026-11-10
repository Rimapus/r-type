/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/PowerUpType
** File description:
** PowerUpType header file
*/

#pragma once

#include <cstdint>

namespace jtype {

/**
 * @brief What a power-up gives the ship that takes it. A value is a byte of the PickUpEvent: a new one is added at the
 *        end, so the others keep their meaning.
 */
enum class PowerUpType : std::uint8_t {
    // One more life, up to MAX_LIVES
    ExtraLife = 0,
    // Takes the next hit instead of the ship
    Shield = 1,
    // The spread shot, until the ship is destroyed
    SpreadShot = 2
};

} // namespace jtype

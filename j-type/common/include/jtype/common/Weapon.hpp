/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Weapon
** File description:
** Weapon header file
*/

#pragma once

#include <cstdint>

namespace jtype {

/**
 * @brief The weapon of a ship. A value is a byte of the scoreboard: a new one is added at the end, so the others keep
 *        their meaning.
 */
enum class Weapon : std::uint8_t {
    // One missile straight ahead, the weapon of a new ship
    Plasma = 0,
    // Three pellets in a fan, given by a SpreadShot power-up
    Spread = 1
};

} // namespace jtype

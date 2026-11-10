/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/Player
** File description:
** Player header file
*/

#pragma once

#include <cstdint>

#include "jtype/common/Weapon.hpp"

namespace jtype {

/**
 * @brief A ship controlled by a client.
 */
struct Player
{
    // From 0 to MAX_PLAYERS - 1
    std::uint8_t slot = 0;
    // The last input bitmask received from the client, see Input.hpp
    std::uint8_t inputs = 0;
    // The time left before the next shot, in seconds
    float shootCooldown = 0.0F;
    // The time left during which nothing hurts the ship, in seconds, e.g. after it came back
    float invulnerable = 0.0F;
    // Each destruction of the ship takes one: it comes back while some are left, and leaves the game at 0
    std::uint8_t lives = 3;
    // Lost with a life: the ship comes back with the plasma
    Weapon weapon = Weapon::Plasma;
};

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Constants
** File description:
** Constants header file
*/

#pragma once

#include <cstddef>
#include <cstdint>

namespace jtype {

// The size of the game area, in pixels of the sprites: the client draws it scaled up
inline constexpr float WORLD_WIDTH = 480.0F;
inline constexpr float WORLD_HEIGHT = 270.0F;

// The number of players in a game, their slots go from 0 to MAX_PLAYERS - 1
inline constexpr std::size_t MAX_PLAYERS = 4;

// The slot written in an event for an entity no player touched, e.g. a Bydo crashing into a ship
inline constexpr std::uint8_t NO_PLAYER = 0xFF;

// The duration of one step of the game, in seconds, on both sides
inline constexpr float TICK_STEP = 1.0F / 60.0F;

} // namespace jtype

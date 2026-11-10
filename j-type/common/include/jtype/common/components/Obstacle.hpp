/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/Obstacle
** File description:
** Obstacle header file
*/

#pragma once

#include <cstdint>

namespace jtype {

/**
 * @brief What an obstacle is: the ceiling or the floor, a block of a wall, or a part of a zapper.
 */
enum class ObstacleKind : std::uint8_t {
    // The ceiling and the floor, which stay along the whole game and are not sent to the clients: they draw them
    Terrain,
    // A block of a wall, broken by the missiles of the players
    Block,
    // The two ends of a zapper, and the beam between them: nothing breaks them
    ZapperNode,
    ZapperBeam,
};

/**
 * @brief Marks an obstacle of the stage: a ship that touches it loses a life, and it stops every missile. Only a block
 *        has a Health, and breaks.
 */
struct Obstacle
{
    ObstacleKind kind = ObstacleKind::Terrain;
};

} // namespace jtype

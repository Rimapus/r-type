/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/systems/PlayerMovementSystem
** File description:
** PlayerMovementSystem header file
*/

#pragma once

#include <cstdint>
#include <vector>

#include "younity/core/Vector2.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace jtype {

/**
 * @brief Move a ship by its arrow inputs for the duration of a step, at PLAYER_SPEED pixels per second in any
 *        direction, then put it back inside the game area. The server moves the ships with it, and the client its own
 *        ship ahead of the server.
 *
 * @param position Where the ship is
 * @param inputs The input bitmask, see Input.hpp
 * @param dt The duration of the step, in seconds
 *
 * @return Where the ship is after the step.
 */
[[nodiscard]] younity::Vector2f moveShip(younity::Vector2f position, std::uint8_t inputs, float dt);

/**
 * @brief Moves the ships of the players with their arrow inputs, see moveShip().
 *        Add it with registry.addSystem<PlayerMovementSystem, Player, Transform>().
 */
class PlayerMovementSystem : public younity::ecs::System
{
public:
    /**
     * @brief Move each ship by its inputs for the duration of the step, then put it back inside the game area.
     *
     * @param registry The registry owning the ships
     * @param entities The entities having a Player and a Transform
     * @param dt The duration of the step, in seconds
     */
    void update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt) override;
};

} // namespace jtype

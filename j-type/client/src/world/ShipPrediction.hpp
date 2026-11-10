/*
** EPITECH PROJECT, 2026
** j-type/client/src/world/ShipPrediction
** File description:
** ShipPrediction header file
*/

#pragma once

#include <cstdint>
#include <deque>
#include <optional>

#include "jtype/common/Messages.hpp"
#include "younity/core/Vector2.hpp"

namespace jtype {

/**
 * @brief The ship of the player, moved at once by its inputs instead of a round trip later: from the newest position
 *        the server gives, the inputs sent since are played again, see moveShip().
 */
class ShipPrediction
{
public:
    /**
     * @brief Move the ship by the inputs just sent, before the server does.
     *
     * @param sequence Their number, from Connection::sendInputs()
     * @param inputs The input bitmask
     * @param step The duration of the step, in seconds
     */
    void move(std::uint32_t sequence, std::uint8_t inputs, float step);

    /**
     * @brief Put the ship back where a snapshot newer than the others has it, then move it by the inputs its ship has
     *        not moved with yet. An older snapshot is dropped.
     *
     * @param snapshot A snapshot received
     * @param slot The slot of this client
     */
    void correct(const SnapshotMessage &snapshot, std::uint8_t slot);

    /**
     * @brief Put the ship of the player where it is predicted, in the world drawn. Nothing changes while the newest
     *        snapshot has no ship of the player.
     *
     * @param world The world to draw, from SnapshotBuffer::step()
     * @param slot The slot of this client
     */
    void place(SnapshotMessage &world, std::uint8_t slot) const;

private:
    /**
     * @brief Inputs sent, played again until a snapshot says the ship moved with them.
     */
    struct Sent
    {
        std::uint32_t sequence;
        std::uint8_t inputs;
        float step;
    };

    // In the order they were sent
    std::deque<Sent> _sent;
    // The tick of the newest snapshot, empty before the first one
    std::optional<std::uint32_t> _tick;
    // Empty while the newest snapshot has no ship of the player
    std::optional<younity::Vector2f> _position;
};

} // namespace jtype

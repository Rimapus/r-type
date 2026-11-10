/*
** EPITECH PROJECT, 2026
** j-type/client/src/world/ShipPrediction
** File description:
** ShipPrediction source file
*/

#include "world/ShipPrediction.hpp"

#include <algorithm>

#include "jtype/common/EntityType.hpp"
#include "jtype/common/systems/PlayerMovementSystem.hpp"

namespace jtype {

void ShipPrediction::move(std::uint32_t sequence, std::uint8_t inputs, float step)
{
    this->_sent.push_back(Sent{sequence, inputs, step});
    if (this->_position) {
        this->_position = moveShip(*this->_position, inputs, step);
    }
}

void ShipPrediction::correct(const SnapshotMessage &snapshot, std::uint8_t slot)
{
    const auto ship = std::ranges::find(snapshot.entities, static_cast<EntityType>(slot), &EntityState::type);

    // A late snapshot would take the ship back
    if (this->_tick && snapshot.tick <= *this->_tick) {
        return;
    }
    this->_tick = snapshot.tick;
    while (!this->_sent.empty() && this->_sent.front().sequence <= snapshot.lastInput) {
        this->_sent.pop_front();
    }
    if (ship == snapshot.entities.end()) {
        this->_position.reset();
        return;
    }
    this->_position = younity::Vector2f{ship->x, ship->y};
    for (const Sent &sent : this->_sent) {
        this->_position = moveShip(*this->_position, sent.inputs, sent.step);
    }
}

void ShipPrediction::place(SnapshotMessage &world, std::uint8_t slot) const
{
    const auto ship = std::ranges::find(world.entities, static_cast<EntityType>(slot), &EntityState::type);

    if (this->_position && ship != world.entities.end()) {
        ship->x = this->_position->x;
        ship->y = this->_position->y;
    }
}

} // namespace jtype

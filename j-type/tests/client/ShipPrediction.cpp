/*
** EPITECH PROJECT, 2026
** j-type/tests/client/ShipPrediction
** File description:
** Unit tests of the ship of the player moved ahead of the server
*/

#include <criterion/criterion.h>

#include <cstdint>

#include "jtype/common/EntityType.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Input.hpp"
#include "jtype/common/Messages.hpp"
#include "world/ShipPrediction.hpp"

using jtype::EntityState;
using jtype::EntityType;
using jtype::ShipPrediction;
using jtype::SnapshotMessage;

namespace {

constexpr float STEP = 0.1F;
constexpr float MOVE = jtype::PLAYER_SPEED * STEP;
// Where the world drawn, a few steps behind, has the ship
constexpr float DRAWN_X = 42.0F;

// A snapshot of the server with the ship of the first slot
SnapshotMessage shipAt(std::uint32_t tick, std::uint32_t lastInput, float x)
{
    SnapshotMessage snapshot;

    snapshot.tick = tick;
    snapshot.lastInput = lastInput;
    snapshot.entities.push_back(EntityState{1, EntityType::Player1, x, 100.0F});
    return snapshot;
}

// The x the ship of the first slot is drawn at
float placedX(const ShipPrediction &prediction)
{
    SnapshotMessage world = shipAt(0, 0, DRAWN_X);

    prediction.place(world, 0);
    return world.entities[0].x;
}

} // namespace

Test(client_ship_prediction, moves_the_ship_at_once)
{
    ShipPrediction prediction;

    prediction.correct(shipAt(1, 0, 100.0F), 0);
    prediction.move(1, jtype::input::RIGHT, STEP);

    const float x = placedX(prediction);

    cr_assert_float_eq(x, 100.0F + MOVE, 1e-3F);
}

Test(client_ship_prediction, plays_again_the_inputs_the_server_has_not_moved_with)
{
    ShipPrediction prediction;

    prediction.correct(shipAt(1, 0, 100.0F), 0);
    prediction.move(1, jtype::input::RIGHT, STEP);
    prediction.move(2, jtype::input::RIGHT, STEP);
    // The server moved the ship with the first input only, and has it at 110 instead of 115
    prediction.correct(shipAt(2, 1, 110.0F), 0);

    const float x = placedX(prediction);

    cr_assert_float_eq(x, 110.0F + MOVE, 1e-3F);
}

Test(client_ship_prediction, drops_a_late_snapshot)
{
    ShipPrediction prediction;

    prediction.correct(shipAt(5, 0, 100.0F), 0);
    prediction.correct(shipAt(4, 0, 50.0F), 0);

    const float x = placedX(prediction);

    cr_assert_float_eq(x, 100.0F, 1e-3F);
}

Test(client_ship_prediction, places_nothing_without_the_ship_of_the_player)
{
    ShipPrediction prediction;
    SnapshotMessage other = shipAt(1, 0, 100.0F);

    const float before = placedX(prediction);

    // Only the ship of the second slot is in the game
    other.entities[0].type = EntityType::Player2;
    prediction.correct(other, 0);
    prediction.move(1, jtype::input::RIGHT, STEP);

    const float without = placedX(prediction);

    cr_assert_float_eq(before, DRAWN_X, 1e-3F);
    cr_assert_float_eq(without, DRAWN_X, 1e-3F);
}

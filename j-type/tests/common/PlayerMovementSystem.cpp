/*
** EPITECH PROJECT, 2026
** j-type/tests/common/PlayerMovementSystem
** File description:
** PlayerMovementSystem unit tests
*/

#include <criterion/criterion.h>

#include <cmath>
#include <cstdint>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Input.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/systems/PlayerMovementSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"

using jtype::moveShip;
using jtype::Player;
using jtype::PlayerMovementSystem;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;

namespace {

// A ship in the middle of the game area, holding the given inputs
Entity createShip(Registry &registry, std::uint8_t inputs, Vector2f position = {200.0F, 100.0F})
{
    const Entity ship = registry.createEntity();

    registry.addSystem<PlayerMovementSystem, Player, Transform>();
    registry.addComponent(ship, Transform{position});
    registry.addComponent(ship, Player{0, inputs});
    return ship;
}

Vector2f positionOf(const Registry &registry, Entity ship)
{
    return registry.getComponent<Transform>(ship).position;
}

} // namespace

Test(common_player_movement_system, stays_still_without_input)
{
    Registry registry;
    const Entity ship = createShip(registry, 0);

    registry.updateSystems(0.5F);
    cr_assert(positionOf(registry, ship) == (Vector2f{200.0F, 100.0F}));
}

Test(common_player_movement_system, moves_at_the_player_speed)
{
    Registry registry;
    const Entity ship = createShip(registry, jtype::input::RIGHT);

    registry.updateSystems(0.5F);
    cr_assert_float_eq(positionOf(registry, ship).x, 200.0F + (jtype::PLAYER_SPEED * 0.5F), 1e-3F);
    cr_assert_float_eq(positionOf(registry, ship).y, 100.0F, 1e-3F);
}

Test(common_player_movement_system, moves_up_and_left)
{
    Registry registry;
    const Entity up = createShip(registry, jtype::input::UP);
    const Entity left = createShip(registry, jtype::input::LEFT);

    registry.updateSystems(0.1F);
    cr_assert_lt(positionOf(registry, up).y, 100.0F);
    cr_assert_lt(positionOf(registry, left).x, 200.0F);
}

Test(common_player_movement_system, diagonal_is_as_fast_as_a_straight_line)
{
    Registry registry;
    const Entity ship = createShip(registry, jtype::input::DOWN | jtype::input::RIGHT);

    registry.updateSystems(0.1F);

    const Vector2f moved = positionOf(registry, ship) - Vector2f{200.0F, 100.0F};

    cr_assert_float_eq(std::hypot(moved.x, moved.y), jtype::PLAYER_SPEED * 0.1F, 1e-3F);
}

Test(common_player_movement_system, opposite_inputs_cancel_out)
{
    Registry registry;
    const Entity ship = createShip(registry, jtype::input::LEFT | jtype::input::RIGHT);

    registry.updateSystems(0.5F);
    cr_assert(positionOf(registry, ship) == (Vector2f{200.0F, 100.0F}));
}

Test(common_player_movement_system, stays_inside_the_game_area)
{
    Registry registry;
    const Entity left = createShip(registry, jtype::input::LEFT | jtype::input::UP, Vector2f{30.0F, 25.0F});
    const Entity right = createShip(registry, jtype::input::RIGHT | jtype::input::DOWN, Vector2f{470.0F, 260.0F});

    registry.updateSystems(1.0F);
    cr_assert(positionOf(registry, left) == jtype::PLAYER_ORIGIN);
    cr_assert(positionOf(registry, right) == (Vector2f{jtype::WORLD_WIDTH - jtype::PLAYER_SIZE.x + jtype::PLAYER_ORIGIN.x,
                                                       jtype::WORLD_HEIGHT - jtype::PLAYER_SIZE.y + jtype::PLAYER_ORIGIN.y}));
}

Test(common_player_movement_system, move_ship_moves_a_ship_without_a_registry)
{
    const Vector2f moved = moveShip(Vector2f{200.0F, 100.0F}, jtype::input::RIGHT, 0.5F);
    const Vector2f held = moveShip(Vector2f{30.0F, 25.0F}, jtype::input::LEFT | jtype::input::UP, 1.0F);

    cr_assert_float_eq(moved.x, 200.0F + (jtype::PLAYER_SPEED * 0.5F), 1e-3F);
    cr_assert_float_eq(moved.y, 100.0F, 1e-3F);
    cr_assert(held == jtype::PLAYER_ORIGIN);
}

/*
** EPITECH PROJECT, 2026
** j-climb/tests/Elements
** File description:
** Elements unit tests
*/

#include <criterion/criterion.h>

#include <algorithm>

#include "TestGame.hpp"

using jclimb::tests::TestGame;

namespace {

// The y of the robot standing on the ground of a test room, and on a block or a platform one tile above it
constexpr float ON_THE_GROUND = 112.0F;
constexpr float ON_A_TILE = 104.0F - 8.0F;
constexpr float SPAWN_X = 16.0F;

// The row of the robot in sheet.png: 1 and 2 while its dash is ready, 3 and 4 once it is used
bool isDashReady(const TestGame &game)
{
    return game.getSprite("scripts/player.lua").textureRect->top < 3 * 8;
}

} // namespace

Test(Elements, a_spring_throws_the_robot_up)
{
    TestGame game(JCLIMB_TESTS_DIR "/rooms/spring.json");
    float highest = ON_THE_GROUND;

    game.start();
    game.press("right");
    for (int count = 0; count < 60; ++count) {
        game.step(1);
        highest = std::min(highest, game.findPlayer()->y);
    }

    cr_assert_lt(highest, ON_THE_GROUND - 30.0F);
}

Test(Elements, a_crumbling_block_breaks_then_comes_back)
{
    TestGame game(JCLIMB_TESTS_DIR "/rooms/crumble.json");

    game.start();
    cr_assert_float_eq(game.findPlayer()->y, ON_A_TILE, 0.001F);
    game.step(90);
    cr_assert_float_eq(game.findPlayer()->y, ON_THE_GROUND, 0.001F);
    cr_assert_not(game.getSprite("scripts/crumble.lua").visible);
    game.press("right");
    game.step(30);
    game.release("right");
    game.step(150);

    cr_assert(game.getSprite("scripts/crumble.lua").visible);
}

Test(Elements, a_platform_carries_the_robot_along)
{
    TestGame game(JCLIMB_TESTS_DIR "/rooms/platform.json");

    game.start();
    game.step(60);

    cr_assert_gt(game.findPlayer()->x, SPAWN_X + 20.0F);
    cr_assert_float_eq(game.findPlayer()->y, ON_A_TILE, 0.001F);
}

Test(Elements, a_platform_lifts_the_robot)
{
    TestGame game(JCLIMB_TESTS_DIR "/rooms/lift.json");

    game.start();
    game.step(60);

    cr_assert_lt(game.findPlayer()->y, ON_A_TILE - 20.0F);
    cr_assert_float_eq(game.findPlayer()->x, SPAWN_X, 0.001F);
}

Test(Elements, a_battery_gives_the_dash_back_in_the_air)
{
    TestGame game(JCLIMB_TESTS_DIR "/rooms/refill.json");
    bool refilled = false;

    game.start();
    game.tap("jump");
    game.step(3);
    game.press("up");
    game.tap("dash");
    for (int count = 0; count < 40 && !refilled; ++count) {
        game.step(1);
        refilled = !game.getSprite("scripts/refill.lua").visible;
    }

    cr_assert(refilled);
    cr_assert_lt(game.findPlayer()->y, ON_THE_GROUND - 8.0F);
    cr_assert(isDashReady(game));
    game.release("up");
    game.step(180);
    cr_assert(game.getSprite("scripts/refill.lua").visible);
}

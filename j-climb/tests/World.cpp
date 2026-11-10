/*
** EPITECH PROJECT, 2026
** j-climb/tests/World
** File description:
** World unit tests
*/

#include <criterion/criterion.h>

#include <optional>

#include "TestGame.hpp"
#include "younity/core/Vector2.hpp"

using jclimb::tests::FIRST_SPAWN;
using jclimb::tests::TestGame;
using younity::Vector2f;

Test(World, starts_the_robot_in_the_first_room)
{
    TestGame game;

    game.start();

    const std::optional<Vector2f> player = game.findPlayer();

    cr_assert(player.has_value());
    cr_assert_float_eq(player->x, FIRST_SPAWN.x, 0.001F);
    cr_assert_float_eq(player->y, FIRST_SPAWN.y, 0.001F);
}

Test(World, runs_while_right_is_held)
{
    TestGame game;

    game.start();
    game.press("right");
    game.step(20);

    cr_assert_gt(game.findPlayer()->x, FIRST_SPAWN.x + 5.0F);
    cr_assert_float_eq(game.findPlayer()->y, FIRST_SPAWN.y, 0.001F);
}

Test(World, jumps_from_the_ground)
{
    TestGame game;

    game.start();
    game.press("jump");
    game.step(15);

    cr_assert_lt(game.findPlayer()->y, FIRST_SPAWN.y - 10.0F);
}

Test(World, dashes_further_than_it_runs)
{
    TestGame running;
    TestGame dashing;

    running.start();
    dashing.start();
    running.press("right");
    dashing.press("right");
    dashing.press("dash");
    running.step(9);
    dashing.step(9);

    cr_assert_gt(dashing.findPlayer()->x, running.findPlayer()->x + 10.0F);
}

Test(World, starts_again_after_running_into_the_spikes)
{
    TestGame game;
    bool reachedSpikes = false;
    bool startedAgain = false;

    game.start();
    game.press("right");
    for (int count = 0; count < 180 && !startedAgain; ++count) {
        game.step(1);
        const float x = game.findPlayer()->x;

        reachedSpikes = reachedSpikes || x > 40.0F;
        startedAgain = reachedSpikes && x < FIRST_SPAWN.x + 1.0F;
    }
    cr_assert(reachedSpikes);
    cr_assert(startedAgain);
}

/*
** EPITECH PROJECT, 2026
** j-climb/tests/Menus
** File description:
** Menus unit tests
*/

#include <criterion/criterion.h>

#include "TestGame.hpp"

using jclimb::tests::FIRST_SPAWN;
using jclimb::tests::TestGame;

namespace {

// A room with the beacon three tiles right of the robot
const char *const BEACON_ROOM = JCLIMB_TESTS_DIR "/rooms/beacon.json";

} // namespace

Test(Menus, starts_on_the_title_screen)
{
    TestGame game;

    game.step(2);

    cr_assert(game.shows("> START"));
    cr_assert(game.shows("CONTROLS"));
    cr_assert(game.shows("QUIT"));
    cr_assert_not(game.findPlayer().has_value());
}

Test(Menus, starts_a_run_with_jump)
{
    TestGame game;

    game.start();

    cr_assert_not(game.shows("QUIT"));
    cr_assert_float_eq(game.findPlayer()->x, FIRST_SPAWN.x, 0.001F);
}

Test(Menus, lists_the_controls_then_goes_back)
{
    TestGame game;

    game.step(2);
    game.tap("down");
    cr_assert(game.shows("> CONTROLS"));
    game.tap("jump");
    game.step(1);
    cr_assert(game.shows("JUMP TO GO BACK"));
    cr_assert(game.shows("DASH"));
    cr_assert(game.shows("PAUSE"));
    game.tap("jump");
    game.step(1);
    cr_assert(game.shows("> START"));
}

Test(Menus, quits_from_the_title_screen)
{
    TestGame game;

    game.step(2);
    game.tap("up");
    cr_assert(game.shows("> QUIT"));
    game.tap("jump");

    cr_assert(game.hasQuit());
}

Test(Menus, pause_stops_the_robot_until_resumed)
{
    TestGame game;

    game.start();
    game.tap("pause");
    cr_assert(game.shows("> RESUME"));
    game.press("right");
    game.step(20);
    cr_assert_float_eq(game.findPlayer()->x, FIRST_SPAWN.x, 0.001F);
    game.tap("pause");
    game.step(20);

    cr_assert_not(game.shows("RESUME"));
    cr_assert_gt(game.findPlayer()->x, FIRST_SPAWN.x + 5.0F);
}

Test(Menus, the_pause_hides_the_name_of_the_room)
{
    TestGame game;

    game.start();
    cr_assert(game.shows("THE YARD"));
    game.tap("pause");

    cr_assert(game.shows("> RESUME"));
    cr_assert_not(game.shows("THE YARD"));
}

Test(Menus, restarts_the_room_from_the_pause)
{
    TestGame game;

    game.start();
    game.press("right");
    game.step(20);
    game.release("right");
    cr_assert_gt(game.findPlayer()->x, FIRST_SPAWN.x + 5.0F);
    game.tap("pause");
    game.tap("down");
    cr_assert(game.shows("> RESTART THE ROOM"));
    game.tap("jump");
    game.step(2);

    cr_assert_float_eq(game.findPlayer()->x, FIRST_SPAWN.x, 0.001F);
}

Test(Menus, goes_back_to_the_title_from_the_pause)
{
    TestGame game;

    game.start();
    game.tap("pause");
    game.tap("up");
    cr_assert(game.shows("> BACK TO THE TITLE"));
    game.tap("jump");
    game.step(2);

    cr_assert(game.shows("> START"));
    cr_assert_not(game.findPlayer().has_value());
}

Test(Menus, shows_the_results_at_the_beacon_then_the_title)
{
    TestGame game(BEACON_ROOM);

    game.start();
    game.press("right");
    game.step(60);
    game.release("right");

    cr_assert(game.shows("BEACON ON!"));
    cr_assert(game.shows("DEATHS 0"));
    cr_assert(game.shows("GEARS 0/0"));
    game.step(60);
    game.tap("jump");
    game.step(1);
    cr_assert(game.shows("> START"));
    cr_assert_not(game.findPlayer().has_value());
}

Test(Menus, the_clock_stops_during_the_pause)
{
    TestGame game(BEACON_ROOM);

    game.start();
    game.tap("pause");
    game.step(180);
    game.tap("pause");
    game.press("right");
    game.step(60);

    cr_assert(game.shows("TIME 00:00."));
}

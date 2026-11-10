/*
** EPITECH PROJECT, 2026
** j-type/tests/client/KonamiCode
** File description:
** Unit tests of the Konami code of the main menu
*/

#include <criterion/criterion.h>

#include <initializer_list>

#include "input/Controls.hpp"
#include "scenes/KonamiCode.hpp"

using jtype::KonamiCode;
using jtype::MenuAction;

namespace {

// Enters the actions, and tells if the last one ended the code
bool enter(KonamiCode &code, std::initializer_list<MenuAction> actions)
{
    bool found = false;

    for (const MenuAction action : actions) {
        found = code.enter(action);
    }
    return found;
}

constexpr MenuAction UP = MenuAction::Up;
constexpr MenuAction DOWN = MenuAction::Down;
constexpr MenuAction LEFT = MenuAction::Left;
constexpr MenuAction RIGHT = MenuAction::Right;

} // namespace

Test(client_konami_code, is_found_once_entered)
{
    KonamiCode code;

    cr_assert_not(enter(code, {UP, UP, DOWN, DOWN, LEFT, RIGHT, LEFT}));
    cr_assert(code.enter(RIGHT));
}

Test(client_konami_code, is_not_found_with_a_wrong_action)
{
    KonamiCode code;

    cr_assert_not(enter(code, {UP, UP, DOWN, DOWN, LEFT, RIGHT, MenuAction::Confirm, RIGHT}));
}

Test(client_konami_code, is_found_after_other_actions)
{
    KonamiCode code;

    cr_assert(enter(code, {UP, UP, UP, DOWN, DOWN, LEFT, RIGHT, LEFT, RIGHT}));
}

Test(client_konami_code, starts_again_from_nothing_once_found)
{
    KonamiCode code;

    cr_assert(enter(code, {UP, UP, DOWN, DOWN, LEFT, RIGHT, LEFT, RIGHT}));
    cr_assert_not(enter(code, {UP, UP, DOWN, DOWN, LEFT, RIGHT, LEFT}));
    cr_assert(code.enter(RIGHT));
}

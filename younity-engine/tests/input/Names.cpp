/*
** EPITECH PROJECT, 2026
** younity-engine/tests/input/Names
** File description:
** Names unit tests
*/

#include <criterion/criterion.h>

#include <optional>
#include <string>
#include <vector>

#include "younity/input/InputManager.hpp"
#include "younity/input/Names.hpp"

using younity::input::AxisDirection;
using younity::input::GamepadAxis;
using younity::input::InputManager;
using younity::input::Key;

Test(input_names, every_key_is_found_by_its_name)
{
    for (int index = static_cast<int>(Key::A); index <= static_cast<int>(Key::F12); ++index) {
        const Key key = static_cast<Key>(index);

        cr_assert(younity::input::parseKey(younity::input::toString(key)) == key);
    }
}

Test(input_names, keys_are_named_like_the_enum)
{
    cr_assert(younity::input::toString(Key::Space) == "Space");
    cr_assert(younity::input::toString(Key::Num1) == "Num1");
    cr_assert(younity::input::parseKey("LShift") == Key::LShift);
}

Test(input_names, unknown_key_name_is_not_found)
{
    cr_assert_not(younity::input::parseKey("Spcae").has_value());
    cr_assert_not(younity::input::parseKey("").has_value());
    cr_assert_not(younity::input::parseKey("space").has_value());
}

Test(input_names, every_axis_is_found_by_its_name)
{
    for (int index = static_cast<int>(GamepadAxis::X); index <= static_cast<int>(GamepadAxis::PovY); ++index) {
        const GamepadAxis axis = static_cast<GamepadAxis>(index);

        cr_assert(younity::input::parseGamepadAxis(younity::input::toString(axis)) == axis);
    }
    cr_assert_not(younity::input::parseGamepadAxis("W").has_value());
}

Test(input_names, describes_keys_then_buttons_then_axes)
{
    const InputManager::Bindings bindings{
        {Key::Space, Key::C},
        {0},
        {{GamepadAxis::X, AxisDirection::Negative}, {GamepadAxis::PovY, AxisDirection::Positive}},
    };

    cr_assert(younity::input::describe(bindings) ==
              (std::vector<std::string>{"Space", "C", "Button 0", "X-", "PovY+"}));
}

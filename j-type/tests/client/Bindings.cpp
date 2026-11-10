/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Bindings
** File description:
** Unit tests of the bindings of the actions to the keys and the gamepad inputs
*/

#include <criterion/criterion.h>

#include <cstddef>
#include <optional>
#include <string>

#include "input/Bindings.hpp"
#include "younity/input/GamepadAxis.hpp"
#include "younity/input/Key.hpp"

using jtype::AxisSide;
using jtype::Bindings;
using jtype::GameAction;
using jtype::GamepadInput;
using younity::input::AxisDirection;
using younity::input::GamepadAxis;
using younity::input::Key;

namespace {

const jtype::Binding &bindingOf(const Bindings &bindings, GameAction action)
{
    return bindings[static_cast<std::size_t>(action)];
}

} // namespace

Test(client_bindings, defaults_are_the_arrows_space_and_the_left_stick)
{
    const Bindings bindings = jtype::getDefaultBindings();

    cr_assert(bindingOf(bindings, GameAction::Up).key == Key::Up);
    cr_assert(bindingOf(bindings, GameAction::Shoot).key == Key::Space);
    cr_assert(bindingOf(bindings, GameAction::Mute).key == Key::M);
    cr_assert(bindingOf(bindings, GameAction::Left).gamepad ==
              GamepadInput(AxisSide{GamepadAxis::X, AxisDirection::Negative}));
    cr_assert(bindingOf(bindings, GameAction::Shoot).gamepad == GamepadInput(0U));
}

Test(client_bindings, every_key_has_a_name_read_back)
{
    for (std::size_t index = 0; index <= static_cast<std::size_t>(Key::F12); ++index) {
        const auto key = static_cast<Key>(index);

        cr_assert(jtype::parseKey(jtype::getKeyName(key)) == key);
    }
    cr_assert(jtype::getKeyName(Key::Space) == "SPACE");
    cr_assert_not(jtype::parseKey("SPACEBAR").has_value());
}

Test(client_bindings, gamepad_inputs_have_a_name_read_back)
{
    const GamepadInput button(12U);
    const GamepadInput axis(AxisSide{GamepadAxis::PovX, AxisDirection::Positive});

    cr_assert(jtype::getGamepadName(button) == "B12");
    cr_assert(jtype::getGamepadName(axis) == "POVX+");
    cr_assert(jtype::parseGamepad("B12") == button);
    cr_assert(jtype::parseGamepad("POVX+") == axis);
    cr_assert_not(jtype::parseGamepad("B32").has_value());
    cr_assert_not(jtype::parseGamepad("W+").has_value());
    cr_assert_not(jtype::parseGamepad("Y").has_value());
}

Test(client_bindings, escape_and_the_back_button_are_kept_for_going_back)
{
    cr_assert(jtype::isReserved(Key::Escape));
    cr_assert_not(jtype::isReserved(Key::Enter));
    cr_assert(jtype::isReserved(GamepadInput(jtype::BACK_BUTTON)));
    cr_assert_not(jtype::isReserved(GamepadInput(0U)));
}

Test(client_bindings, the_keys_of_the_layouts_are_kept_for_the_layouts)
{
    cr_assert(jtype::isReserved(jtype::DEBUG_LAYOUT_KEY));
    cr_assert(jtype::isReserved(jtype::EDITOR_LAYOUT_KEY));
    cr_assert_not(jtype::isReserved(Key::F3));
}

Test(client_bindings, a_key_taken_from_another_action_swaps_with_it)
{
    Bindings bindings = jtype::getDefaultBindings();

    // Shoot takes the key of Up, so Up gets the old key of Shoot
    jtype::rebindKey(bindings, GameAction::Shoot, Key::Up);
    cr_assert(bindingOf(bindings, GameAction::Shoot).key == Key::Up);
    cr_assert(bindingOf(bindings, GameAction::Up).key == Key::Space);
    jtype::rebindKey(bindings, GameAction::Mute, Key::P);
    cr_assert(bindingOf(bindings, GameAction::Mute).key == Key::P);
}

Test(client_bindings, a_gamepad_input_taken_from_another_action_swaps_with_it)
{
    Bindings bindings = jtype::getDefaultBindings();
    const GamepadInput shoot = bindingOf(bindings, GameAction::Shoot).gamepad;

    jtype::rebindGamepad(bindings, GameAction::Mute, shoot);
    cr_assert(bindingOf(bindings, GameAction::Mute).gamepad == shoot);
    cr_assert(bindingOf(bindings, GameAction::Shoot).gamepad == GamepadInput(3U));
}

Test(client_bindings, an_action_is_written_with_its_key_and_its_gamepad_input)
{
    const Bindings bindings = jtype::getDefaultBindings();

    cr_assert_eq(jtype::formatBinding(GameAction::Shoot, bindingOf(bindings, GameAction::Shoot)), "SHOOT: SPACE / B0");
}

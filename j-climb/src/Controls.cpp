/*
** EPITECH PROJECT, 2026
** j-climb/src/Controls
** File description:
** Controls source file
*/

#include "Controls.hpp"

#include <optional>
#include <string>

#include "younity/input/GamepadAxis.hpp"
#include "younity/input/Key.hpp"
#include "younity/input/Names.hpp"

namespace jclimb {

younity::scripting::Actions getActions(younity::input::InputManager &input)
{
    return younity::scripting::Actions{
        .isPressed = [&input](const std::string &action) { return input.isActionPressed(action); },
        .isJustPressed = [&input](const std::string &action) { return input.isActionJustPressed(action); },
        .isJustReleased = [&input](const std::string &action) { return input.isActionJustReleased(action); },
        .bindKey =
            [&input](const std::string &action, const std::string &name) {
                const std::optional<younity::input::Key> key = younity::input::parseKey(name);

                if (key) {
                    input.bindKey(action, *key);
                }
                return key.has_value();
            },
        .bindButton = [&input](const std::string &action, unsigned int button) { input.bindGamepadButton(action, button); },
        .bindAxis =
            [&input](const std::string &action, const std::string &name, bool positive) {
                const std::optional<younity::input::GamepadAxis> axis = younity::input::parseGamepadAxis(name);

                if (axis) {
                    input.bindGamepadAxis(action, *axis,
                                          positive ? younity::input::AxisDirection::Positive
                                                   : younity::input::AxisDirection::Negative);
                }
                return axis.has_value();
            },
        .unbind = [&input](const std::string &action) { input.unbind(action); },
        .getBindings = [&input](const std::string &action) { return younity::input::describe(input.getBindings(action)); },
    };
}

} // namespace jclimb

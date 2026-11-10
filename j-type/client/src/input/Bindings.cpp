/*
** EPITECH PROJECT, 2026
** j-type/client/src/input/Bindings
** File description:
** Bindings source file
*/

#include "input/Bindings.hpp"

#include <charconv>
#include <format>
#include <system_error>

namespace jtype {

namespace {

using younity::input::AxisDirection;
using younity::input::GamepadAxis;
using younity::input::Key;

// In the order of Key; only the characters of the 8x8 font, so the controls screen can write them
constexpr std::array<std::string_view, 63> KEY_NAMES{
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "LEFT", "RIGHT", "UP", "DOWN", "SPACE", "ENTER", "ESCAPE", "BACKSPACE", "TAB",
    "LSHIFT", "RSHIFT", "LCTRL", "RCTRL", "LALT", "RALT",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"};

static_assert(KEY_NAMES.size() == static_cast<std::size_t>(Key::F12) + 1);

// In the order of GamepadAxis
constexpr std::array<std::string_view, 8> AXIS_NAMES{"X", "Y", "Z", "R", "U", "V", "POVX", "POVY"};

static_assert(AXIS_NAMES.size() == static_cast<std::size_t>(GamepadAxis::PovY) + 1);

// In the order of GameAction
constexpr std::array<std::string_view, GAME_ACTION_COUNT> ACTION_NAMES{"UP", "DOWN", "LEFT", "RIGHT", "SHOOT", "MUTE"};

/**
 * @brief Get the binding of an action.
 *
 * @param bindings The bindings
 * @param action The action
 *
 * @return Its binding.
 */
Binding &getBinding(Bindings &bindings, GameAction action)
{
    return bindings[static_cast<std::size_t>(action)];
}

} // namespace

Bindings getDefaultBindings(void)
{
    return Bindings{
        {
         {Key::Up, AxisSide{GamepadAxis::Y, AxisDirection::Negative}},
         {Key::Down, AxisSide{GamepadAxis::Y, AxisDirection::Positive}},
         {Key::Left, AxisSide{GamepadAxis::X, AxisDirection::Negative}},
         {Key::Right, AxisSide{GamepadAxis::X, AxisDirection::Positive}},
         {Key::Space, 0U},
         {Key::M, 3U},
         }
    };
}

std::string_view getActionName(GameAction action)
{
    return ACTION_NAMES[static_cast<std::size_t>(action)];
}

std::string_view getKeyName(Key key)
{
    return KEY_NAMES[static_cast<std::size_t>(key)];
}

std::optional<Key> parseKey(std::string_view name)
{
    for (std::size_t key = 0; key < KEY_NAMES.size(); ++key) {
        if (KEY_NAMES[key] == name) {
            return static_cast<Key>(key);
        }
    }
    return std::nullopt;
}

std::string getGamepadName(const GamepadInput &input)
{
    if (const auto *button = std::get_if<unsigned int>(&input)) {
        return std::format("B{}", *button);
    }

    const AxisSide &side = std::get<AxisSide>(input);

    return std::format("{}{}", AXIS_NAMES[static_cast<std::size_t>(side.axis)],
                       side.direction == AxisDirection::Negative ? '-' : '+');
}

std::string formatBinding(GameAction action, const Binding &binding)
{
    return std::format("{}: {} / {}", getActionName(action), getKeyName(binding.key), getGamepadName(binding.gamepad));
}

std::optional<GamepadInput> parseGamepad(std::string_view name)
{
    if (name.size() < 2) {
        return std::nullopt;
    }
    if (name.front() == 'B') {
        unsigned int button = 0;
        const auto [end, error] = std::from_chars(name.data() + 1, name.data() + name.size(), button);

        if (error != std::errc() || end != name.data() + name.size() || button >= GAMEPAD_BUTTONS) {
            return std::nullopt;
        }
        return GamepadInput(button);
    }

    const std::string_view axis = name.substr(0, name.size() - 1);
    const char side = name.back();

    if (side != '-' && side != '+') {
        return std::nullopt;
    }
    for (std::size_t index = 0; index < AXIS_NAMES.size(); ++index) {
        if (AXIS_NAMES[index] == axis) {
            return GamepadInput(AxisSide{static_cast<GamepadAxis>(index),
                                         side == '-' ? AxisDirection::Negative : AxisDirection::Positive});
        }
    }
    return std::nullopt;
}

bool isReserved(Key key)
{
    return key == Key::Escape || key == DEBUG_LAYOUT_KEY || key == EDITOR_LAYOUT_KEY;
}

bool isReserved(const GamepadInput &input)
{
    return input == GamepadInput(BACK_BUTTON);
}

void rebindKey(Bindings &bindings, GameAction action, Key key)
{
    Binding &target = getBinding(bindings, action);

    for (Binding &other : bindings) {
        if (&other != &target && other.key == key) {
            other.key = target.key;
        }
    }
    target.key = key;
}

void rebindGamepad(Bindings &bindings, GameAction action, const GamepadInput &input)
{
    Binding &target = getBinding(bindings, action);

    for (Binding &other : bindings) {
        if (&other != &target && other.gamepad == input) {
            other.gamepad = target.gamepad;
        }
    }
    target.gamepad = input;
}

} // namespace jtype

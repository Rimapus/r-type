/*
** EPITECH PROJECT, 2026
** j-type/client/src/input/Controls
** File description:
** Controls source file
*/

#include "input/Controls.hpp"

#include <array>
#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "jtype/common/Input.hpp"
#include "younity/input/GamepadAxis.hpp"
#include "younity/input/SfmlInputSource.hpp"

namespace jtype {

namespace {

using younity::input::AxisDirection;
using younity::input::GamepadAxis;
using younity::input::Key;

// The names the actions of the game are bound under, in the order of GameAction
constexpr std::array<const char *, GAME_ACTION_COUNT> GAME_ACTIONS{"up", "down", "left", "right", "shoot", "mute"};

// The bit of each action sent to the server, in the order of GameAction; the sound is only played here
constexpr std::array<std::uint8_t, GAME_ACTION_COUNT> GAME_BITS{input::UP, input::DOWN, input::LEFT, input::RIGHT,
                                                                input::SHOOT, 0};

// In the order of MenuAction
constexpr std::array<const char *, 6> MENU_ACTIONS{
    "menu_up",
    "menu_down",
    "menu_left",
    "menu_right",
    "menu_confirm",
    "menu_back",
};

// The keys, buttons and axes a binding can take, each under its own action so the capture finds the one pressed
constexpr std::size_t KEY_COUNT = static_cast<std::size_t>(Key::F12) + 1;
constexpr std::size_t AXIS_COUNT = static_cast<std::size_t>(GamepadAxis::PovY) + 1;

/**
 * @brief Find the name of an action of the menus.
 *
 * @param action The action
 *
 * @return The name it is bound under.
 */
const char *getMenuActionName(MenuAction action)
{
    return MENU_ACTIONS[static_cast<std::size_t>(action)];
}

/**
 * @brief Find the name of the action of the capture bound to a key.
 *
 * @param key The index of the key in Key
 *
 * @return The name it is bound under.
 */
std::string getKeyActionName(std::size_t key)
{
    return std::format("capture_key_{}", key);
}

/**
 * @brief Find the name of the action of the capture bound to a gamepad input.
 *
 * @param input The gamepad input
 *
 * @return The name it is bound under.
 */
std::string getGamepadActionName(const GamepadInput &input)
{
    return "capture_pad_" + getGamepadName(input);
}

/**
 * @brief List every gamepad input a binding can take: the buttons, then both sides of each axis.
 *
 * @return The inputs.
 */
std::vector<GamepadInput> getGamepadInputs(void)
{
    std::vector<GamepadInput> inputs;

    for (unsigned int button = 0; button < GAMEPAD_BUTTONS; ++button) {
        inputs.emplace_back(button);
    }
    for (std::size_t axis = 0; axis < AXIS_COUNT; ++axis) {
        inputs.emplace_back(AxisSide{static_cast<GamepadAxis>(axis), AxisDirection::Negative});
        inputs.emplace_back(AxisSide{static_cast<GamepadAxis>(axis), AxisDirection::Positive});
    }
    return inputs;
}

} // namespace

Controls::Controls(void)
    : Controls(std::make_unique<younity::input::SfmlInputSource>()) {}

Controls::Controls(std::unique_ptr<younity::input::IInputSource> source)
    : _inputs(std::move(source))
{
    this->apply(getDefaultBindings());
    this->_inputs.bindKey(getMenuActionName(MenuAction::Up), Key::Up);
    this->_inputs.bindKey(getMenuActionName(MenuAction::Down), Key::Down);
    this->_inputs.bindKey(getMenuActionName(MenuAction::Left), Key::Left);
    this->_inputs.bindKey(getMenuActionName(MenuAction::Right), Key::Right);
    this->_inputs.bindKey(getMenuActionName(MenuAction::Confirm), Key::Enter);
    this->_inputs.bindKey(getMenuActionName(MenuAction::Back), Key::Escape);
    // The left stick and not the directional pad: SFML gives its vertical axis a different sign on each system
    this->_inputs.bindGamepadAxis(getMenuActionName(MenuAction::Up), GamepadAxis::Y, AxisDirection::Negative);
    this->_inputs.bindGamepadAxis(getMenuActionName(MenuAction::Down), GamepadAxis::Y, AxisDirection::Positive);
    this->_inputs.bindGamepadAxis(getMenuActionName(MenuAction::Left), GamepadAxis::X, AxisDirection::Negative);
    this->_inputs.bindGamepadAxis(getMenuActionName(MenuAction::Right), GamepadAxis::X, AxisDirection::Positive);
    this->_inputs.bindGamepadButton(getMenuActionName(MenuAction::Confirm), 0);
    this->_inputs.bindGamepadButton(getMenuActionName(MenuAction::Back), BACK_BUTTON);
    for (std::size_t key = 0; key < KEY_COUNT; ++key) {
        this->_inputs.bindKey(getKeyActionName(key), static_cast<Key>(key));
    }
    for (const GamepadInput &input : getGamepadInputs()) {
        const std::string action = getGamepadActionName(input);

        if (const auto *button = std::get_if<unsigned int>(&input)) {
            this->_inputs.bindGamepadButton(action, *button);
        } else {
            this->_inputs.bindGamepadAxis(action, std::get<AxisSide>(input).axis, std::get<AxisSide>(input).direction);
        }
    }
}

void Controls::apply(const Bindings &bindings)
{
    for (std::size_t action = 0; action < GAME_ACTION_COUNT; ++action) {
        const Binding &binding = bindings[action];
        const char *name = GAME_ACTIONS[action];

        this->_inputs.unbind(name);
        this->_inputs.bindKey(name, binding.key);
        if (const auto *button = std::get_if<unsigned int>(&binding.gamepad)) {
            this->_inputs.bindGamepadButton(name, *button);
        } else {
            const AxisSide &side = std::get<AxisSide>(binding.gamepad);

            this->_inputs.bindGamepadAxis(name, side.axis, side.direction);
        }
    }
}

void Controls::update(void)
{
    this->_inputs.update();
}

void Controls::setEnabled(bool isEnabled) noexcept
{
    this->_isEnabled = isEnabled;
}

std::uint8_t Controls::read(void) const
{
    std::uint8_t inputs = 0;

    if (!this->_isEnabled) {
        return inputs;
    }

    for (std::size_t action = 0; action < GAME_ACTION_COUNT; ++action) {
        inputs |= this->_inputs.isActionPressed(GAME_ACTIONS[action]) ? GAME_BITS[action] : 0U;
    }
    return inputs;
}

bool Controls::isMutePressed(void) const
{
    return this->_isEnabled && this->_inputs.isActionJustPressed(GAME_ACTIONS[static_cast<std::size_t>(GameAction::Mute)]);
}

bool Controls::isMenuPressed(MenuAction action) const
{
    return this->_isEnabled && this->_inputs.isActionJustPressed(getMenuActionName(action));
}

std::optional<Key> Controls::getKeyJustPressed(void) const
{
    if (!this->_isEnabled) {
        return std::nullopt;
    }
    for (std::size_t key = 0; key < KEY_COUNT; ++key) {
        if (this->_inputs.isActionJustPressed(getKeyActionName(key))) {
            return static_cast<Key>(key);
        }
    }
    return std::nullopt;
}

std::optional<GamepadInput> Controls::getGamepadJustPressed(void) const
{
    if (!this->_isEnabled) {
        return std::nullopt;
    }
    for (const GamepadInput &input : getGamepadInputs()) {
        if (this->_inputs.isActionJustPressed(getGamepadActionName(input))) {
            return input;
        }
    }
    return std::nullopt;
}

} // namespace jtype

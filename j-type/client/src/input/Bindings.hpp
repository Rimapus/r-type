/*
** EPITECH PROJECT, 2026
** j-type/client/src/input/Bindings
** File description:
** Bindings header file
*/

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "younity/input/GamepadAxis.hpp"
#include "younity/input/Key.hpp"

namespace jtype {

/**
 * @brief An action of the game, bound to a key and to a gamepad input the player can change.
 */
enum class GameAction {
    Up,
    Down,
    Left,
    Right,
    Shoot,
    Mute
};

inline constexpr std::size_t GAME_ACTION_COUNT = 6;

// SFML reads up to 32 buttons per gamepad (sf::Joystick::ButtonCount)
inline constexpr unsigned int GAMEPAD_BUTTONS = 32;

// Goes back in the menus and leaves the game, like Escape: neither can be bound to an action
inline constexpr unsigned int BACK_BUTTON = 1;

// Show the debug and the editor layouts of the engine, see Client: neither can be bound to an action
inline constexpr younity::input::Key DEBUG_LAYOUT_KEY = younity::input::Key::F1;
inline constexpr younity::input::Key EDITOR_LAYOUT_KEY = younity::input::Key::F2;

/**
 * @brief A side of a gamepad axis, pressed when the axis is pushed more than halfway on it.
 */
struct AxisSide
{
    younity::input::GamepadAxis axis;
    younity::input::AxisDirection direction;

    bool operator==(const AxisSide &other) const = default;
};

/**
 * @brief A button of a gamepad, by its number, or a side of one of its axes.
 */
using GamepadInput = std::variant<unsigned int, AxisSide>;

/**
 * @brief What presses an action: a key of the keyboard and an input of a gamepad.
 */
struct Binding
{
    younity::input::Key key;
    GamepadInput gamepad;

    bool operator==(const Binding &other) const = default;
};

/**
 * @brief The binding of each action, in the order of GameAction.
 */
using Bindings = std::array<Binding, GAME_ACTION_COUNT>;

/**
 * @brief Get the bindings of a new player: the arrows and the left stick move, space and the first button shoot, M and
 *        the fourth button mute the sound.
 *
 * @return The default bindings.
 */
[[nodiscard]] Bindings getDefaultBindings(void);

/**
 * @brief Get the name of an action, as the controls screen shows it.
 *
 * @param action The action
 *
 * @return Its name, e.g. "SHOOT".
 */
[[nodiscard]] std::string_view getActionName(GameAction action);

/**
 * @brief Get the name of a key, as the controls screen shows it and the settings file stores it.
 *
 * @param key The key
 *
 * @return Its name, e.g. "A", "UP" or "SPACE".
 */
[[nodiscard]] std::string_view getKeyName(younity::input::Key key);

/**
 * @brief Find a key by its name.
 *
 * @param name The name given by getKeyName()
 *
 * @return The key, or nothing for an unknown name.
 */
[[nodiscard]] std::optional<younity::input::Key> parseKey(std::string_view name);

/**
 * @brief Get the name of a gamepad input, as the controls screen shows it and the settings file stores it.
 *
 * @param input The input
 *
 * @return Its name: B and the number of a button, e.g. "B0", or an axis and its side, e.g. "Y-".
 */
[[nodiscard]] std::string getGamepadName(const GamepadInput &input);

/**
 * @brief Write an action with its key and its gamepad input, as the controls and the help screens show them.
 *
 * @param action The action
 * @param binding Its key and its gamepad input
 *
 * @return The text, e.g. "SHOOT: SPACE / B0".
 */
[[nodiscard]] std::string formatBinding(GameAction action, const Binding &binding);

/**
 * @brief Find a gamepad input by its name.
 *
 * @param name The name given by getGamepadName()
 *
 * @return The input, or nothing for an unknown name.
 */
[[nodiscard]] std::optional<GamepadInput> parseGamepad(std::string_view name);

/**
 * @brief Check if a key is kept for going back or for the layouts, so it cannot be bound to an action.
 *
 * @param key The key
 *
 * @return true for Escape, DEBUG_LAYOUT_KEY and EDITOR_LAYOUT_KEY.
 */
[[nodiscard]] bool isReserved(younity::input::Key key);

/**
 * @brief Check if a gamepad input is kept for going back, so it cannot be bound to an action.
 *
 * @param input The gamepad input
 *
 * @return true for BACK_BUTTON.
 */
[[nodiscard]] bool isReserved(const GamepadInput &input);

/**
 * @brief Bind an action to a key. The action that had the key takes the old key of this one, so no action is left
 *        without a key and no key presses two actions.
 *
 * @param bindings The bindings to change
 * @param action The action
 * @param key Its new key
 */
void rebindKey(Bindings &bindings, GameAction action, younity::input::Key key);

/**
 * @brief Bind an action to a gamepad input. The action that had the input takes the old input of this one.
 *
 * @param bindings The bindings to change
 * @param action The action
 * @param input Its new gamepad input
 */
void rebindGamepad(Bindings &bindings, GameAction action, const GamepadInput &input);

} // namespace jtype

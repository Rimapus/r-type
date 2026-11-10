/*
** EPITECH PROJECT, 2026
** younity-engine/modules/input/include/younity/input/Names
** File description:
** Names header file
*/

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "younity/input/GamepadAxis.hpp"
#include "younity/input/InputManager.hpp"
#include "younity/input/Key.hpp"

namespace younity::input {

/**
 * @brief Get the name of a key, as written in the Key enum, e.g. "Space", "A", "Num1" or "LShift".
 *
 * @param key The key
 *
 * @return Its name.
 */
[[nodiscard]] std::string_view toString(Key key) noexcept;

/**
 * @brief Find a key by its name, as toString() writes it.
 *
 * @param name The name, e.g. "Space"
 *
 * @return The key, or nothing if no key has this name.
 */
[[nodiscard]] std::optional<Key> parseKey(std::string_view name) noexcept;

/**
 * @brief Get the name of a gamepad axis, as written in the GamepadAxis enum, e.g. "X" or "PovY".
 *
 * @param axis The axis
 *
 * @return Its name.
 */
[[nodiscard]] std::string_view toString(GamepadAxis axis) noexcept;

/**
 * @brief Find a gamepad axis by its name, as toString() writes it.
 *
 * @param name The name, e.g. "X"
 *
 * @return The axis, or nothing if no axis has this name.
 */
[[nodiscard]] std::optional<GamepadAxis> parseGamepadAxis(std::string_view name) noexcept;

/**
 * @brief Write what is bound to an action for a player to read, e.g. on a controls screen:
 *        the name of each key, then "Button 0" for a gamepad button and "X-" or "X+" for a side of an axis.
 *
 * @param bindings What is bound to the action, from InputManager::getBindings()
 *
 * @return One name per key, button and axis, in this order.
 */
[[nodiscard]] std::vector<std::string> describe(const InputManager::Bindings &bindings);

} // namespace younity::input

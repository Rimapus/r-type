/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/include/younity/scripting/Actions
** File description:
** Actions header file
*/

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace younity::scripting {

/**
 * @brief What the scripts read from the input of the game, and how they bind it, by the name of an action, e.g.
 *        Actions{[&input](const std::string &action) { return input.isActionPressed(action); }, ...}.
 *        An empty function answers false or nothing, e.g. on a server without input.
 */
struct Actions
{
    /** @brief Tells if the action is held down. */
    std::function<bool(const std::string &)> isPressed;
    /** @brief Tells if the action has just been pressed. */
    std::function<bool(const std::string &)> isJustPressed;
    /** @brief Tells if the action has just been released. */
    std::function<bool(const std::string &)> isJustReleased;
    /** @brief Binds a key to an action by the name of the key, e.g. "Space", and tells if the key exists. */
    std::function<bool(const std::string &action, const std::string &key)> bindKey = {};
    /** @brief Binds a gamepad button to an action by its number. */
    std::function<void(const std::string &action, unsigned int button)> bindButton = {};
    /** @brief Binds a side of a gamepad axis to an action by the name of the axis, e.g. "X", and tells if it exists. */
    std::function<bool(const std::string &action, const std::string &axis, bool positive)> bindAxis = {};
    /** @brief Removes everything bound to an action. */
    std::function<void(const std::string &action)> unbind = {};
    /** @brief Names what is bound to an action for a player to read, e.g. {"Space", "Button 0"}. */
    std::function<std::vector<std::string>(const std::string &action)> getBindings = {};
};

} // namespace younity::scripting

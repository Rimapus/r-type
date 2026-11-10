/*
** EPITECH PROJECT, 2026
** j-climb/src/Controls
** File description:
** Controls header file
*/

#pragma once

#include "younity/input/InputManager.hpp"
#include "younity/scripting/Actions.hpp"

namespace jclimb {

/**
 * @brief Give an input manager to the scripts: they read its actions, bind them by the names of the keys and the axes,
 *        and list what is bound, e.g. the scripts/controls.lua module binds "jump" to "Space".
 *
 * @param input The input manager of the game, it must outlive the actions
 *
 * @return The actions, read from and bound in the input manager by their name.
 */
[[nodiscard]] younity::scripting::Actions getActions(younity::input::InputManager &input);

} // namespace jclimb

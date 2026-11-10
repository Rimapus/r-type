/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Help
** File description:
** Help header file
*/

#pragma once

#include <string>
#include <vector>

#include "input/Bindings.hpp"

namespace jtype {

/**
 * @brief Write the lines of the help screen: the goal of the game, each action with its key and its gamepad input,
 *        then the keys of the menus and of the windows of the engine.
 *
 * @param bindings The bindings of the player, so the help shows the keys they chose
 *
 * @return The lines, from the top; an empty line leaves a gap.
 */
[[nodiscard]] std::vector<std::string> getHelpLines(const Bindings &bindings);

} // namespace jtype

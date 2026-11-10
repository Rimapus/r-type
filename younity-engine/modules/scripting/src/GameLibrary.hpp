/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/GameLibrary
** File description:
** GameLibrary header file, private to the scripting module
*/

#pragma once

#include "younity/scripting/Game.hpp"
#include <sol/sol.hpp>

namespace younity::scripting {

/**
 * @brief Add the game table to the interpreter: the functions a script uses to change scene, quit and set the speed
 *        of the game.
 *
 * @param lua The interpreter
 * @param game The functions of the game the table calls
 */
void openGameLibrary(sol::state &lua, const Game &game);

} // namespace younity::scripting

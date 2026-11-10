/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/InputLibrary
** File description:
** InputLibrary header file, private to the scripting module
*/

#pragma once

#include "younity/scripting/Actions.hpp"
#include <sol/sol.hpp>

namespace younity::scripting {

/**
 * @brief Add the input table to the interpreter: the functions a script uses to check the actions of the game and
 *        to bind them.
 *
 * @param lua The interpreter
 * @param actions The actions the functions read
 */
void openInputLibrary(sol::state &lua, const Actions &actions);

} // namespace younity::scripting

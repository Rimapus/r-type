/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/LogLibrary
** File description:
** LogLibrary header file, private to the scripting module
*/

#pragma once

#include <sol/sol.hpp>

namespace younity::scripting {

/**
 * @brief Add the log table to the interpreter: the functions a script uses to write to the logger of the
 *        scripting module, each message starting with the file and the line of the call.
 *
 * @param lua The interpreter
 */
void openLogLibrary(sol::state &lua);

} // namespace younity::scripting

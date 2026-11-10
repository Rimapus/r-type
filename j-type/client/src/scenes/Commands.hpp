/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Commands
** File description:
** Commands header file
*/

#pragma once

#include "scenes/Context.hpp"
#include "younity/tools/Console.hpp"

namespace jtype {

/**
 * @brief Add the commands of the game to a console: `quit`, `menu`, `mute`, `volume <music|effects> <0-100>`,
 *        `colorblind <off|protanopia|deuteranopia|tritanopia>`, `captions <on|off>`, `autofire <on|off>` and
 *        `contrast <on|off>`. A setting changed by a command is applied at once, the high contrast from the next game
 *        screen, and saved like on the settings screen.
 *
 * @param console The console, e.g. younity::tools::console()
 * @param context What the screens share; it must outlive the commands, see removeCommands()
 *
 * @throw std::invalid_argument if one of the names is already used.
 */
void registerCommands(younity::tools::Console &console, Context &context);

/**
 * @brief Remove the commands added by registerCommands().
 *
 * @param console The console they were added to
 */
void removeCommands(younity::tools::Console &console);

} // namespace jtype

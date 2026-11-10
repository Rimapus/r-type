/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/StorageLibrary
** File description:
** StorageLibrary header file, private to the scripting module
*/

#pragma once

#include <filesystem>
#include <sol/sol.hpp>

namespace younity::scripting {

/**
 * @brief Add the storage table to the interpreter: the functions a script uses to save a table to a JSON file and
 *        load it back, e.g. the best times or the settings.
 *
 * @param lua The interpreter
 * @param saves The folder of the files, created at the first save; empty, nothing is saved or loaded
 */
void openStorageLibrary(sol::state &lua, std::filesystem::path saves);

} // namespace younity::scripting

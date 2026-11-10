/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/AssetPath
** File description:
** AssetPath header file, private to the scripting module
*/

#pragma once

#include <string>

namespace younity::scripting {

/**
 * @brief Check a path a script gives to a file of the assets, e.g. a sound or a scene: it must stay inside them.
 *
 * @param path The path, relative to the assets
 * @param what What the path names, for the error, e.g. "the sound"
 *
 * @throw std::invalid_argument if the path is empty, absolute, or goes up with "..".
 */
void checkAssetPath(const std::string &path, const std::string &what);

} // namespace younity::scripting

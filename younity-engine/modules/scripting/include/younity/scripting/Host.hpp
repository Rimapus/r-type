/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/include/younity/scripting/Host
** File description:
** Host header file
*/

#pragma once

#include <filesystem>

#include "younity/scripting/Actions.hpp"
#include "younity/scripting/Audio.hpp"
#include "younity/scripting/Game.hpp"

namespace younity::scripting {

/**
 * @brief Everything the game gives its scripts besides the registry: its input, its audio, its game loop and the
 *        folder of its saves. The scripting module stays built on core, ecs and physics: the game connects the
 *        functions to its input and audio modules, and leaves out what it does not have.
 */
struct Host
{
    /** @brief The input table of the scripts. */
    Actions actions;
    /** @brief The audio table of the scripts. */
    Audio audio = {};
    /** @brief The game table of the scripts. */
    Game game = {};
    /** @brief The folder the storage table writes in, created when needed; empty, the scripts cannot save. */
    std::filesystem::path saves = {};
};

} // namespace younity::scripting

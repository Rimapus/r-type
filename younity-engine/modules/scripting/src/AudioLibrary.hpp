/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/AudioLibrary
** File description:
** AudioLibrary header file, private to the scripting module
*/

#pragma once

#include "younity/scripting/Audio.hpp"
#include <sol/sol.hpp>

namespace younity::scripting {

/**
 * @brief Add the audio table to the interpreter: the functions a script uses to play sounds and music.
 *
 * @param lua The interpreter
 * @param audio The functions of the game the table calls
 */
void openAudioLibrary(sol::state &lua, const Audio &audio);

} // namespace younity::scripting

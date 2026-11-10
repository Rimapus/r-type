/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/include/younity/scripting/Audio
** File description:
** Audio header file
*/

#pragma once

#include <functional>
#include <string>

namespace younity::scripting {

/**
 * @brief How the scripts play the sounds and the music of the game, e.g. with the SoundPlayer and a Music of the
 *        audio module. The paths are the ones the scripts give, relative to the assets of the game.
 *        An empty function does nothing, e.g. on a server without audio.
 */
struct Audio
{
    /** @brief Plays a sound once, at a volume from 0 to 1. */
    std::function<void(const std::string &path, float volume)> play = {};
    /** @brief Plays a music in place of the one playing, looping or once. */
    std::function<void(const std::string &path, bool looping)> playMusic = {};
    /** @brief Pauses the music: playMusic with the same path starts it again from there. */
    std::function<void(void)> pauseMusic = {};
    /** @brief Stops the music. */
    std::function<void(void)> stopMusic = {};
    /** @brief Sets the volume of all the sounds, from 0 to 1. */
    std::function<void(float volume)> setVolume = {};
    /** @brief Sets the volume of the music, from 0 to 1. */
    std::function<void(float volume)> setMusicVolume = {};
};

} // namespace younity::scripting

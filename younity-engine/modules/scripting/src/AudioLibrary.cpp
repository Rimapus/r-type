/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/AudioLibrary
** File description:
** AudioLibrary source file
*/

#include "AudioLibrary.hpp"
#include "AssetPath.hpp"

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace younity::scripting {

namespace {

/**
 * @brief Read a volume given by a script.
 *
 * @param volume The volume given by the script
 *
 * @return The volume.
 *
 * @throw std::invalid_argument if it is not a number from 0 to 1.
 */
float toVolume(double volume)
{
    // NaN fails both comparisons
    if (!(volume >= 0 && volume <= 1)) {
        throw std::invalid_argument("the volume must be a number from 0 to 1");
    }
    return static_cast<float>(volume);
}

/**
 * @brief Get the Lua function setting a volume.
 *
 * @param set The function of the game, nothing happens if it is empty
 *
 * @return The function, taking the volume.
 */
auto volumeSetterOf(std::function<void(float)> set)
{
    return [set = std::move(set)](double volume) {
        const float checked = toVolume(volume);

        if (set) {
            set(checked);
        }
    };
}

/**
 * @brief Get the Lua function doing something to the music.
 *
 * @param action The function of the game, nothing happens if it is empty
 *
 * @return The function, taking nothing.
 */
auto musicActionOf(std::function<void(void)> action)
{
    return [action = std::move(action)] {
        if (action) {
            action();
        }
    };
}

} // namespace

void openAudioLibrary(sol::state &lua, const Audio &audio)
{
    sol::table library = lua.create_named_table("audio");

    library.set_function("play", [play = audio.play](const std::string &path, sol::optional<double> volume) {
        checkAssetPath(path, "the sound");
        const float checked = toVolume(volume.value_or(1.0));

        if (play) {
            play(path, checked);
        }
    });
    library.set_function("playMusic", [play = audio.playMusic](const std::string &path, sol::optional<bool> looping) {
        checkAssetPath(path, "the music");
        if (play) {
            play(path, looping.value_or(true));
        }
    });
    library.set_function("pauseMusic", musicActionOf(audio.pauseMusic));
    library.set_function("stopMusic", musicActionOf(audio.stopMusic));
    library.set_function("setVolume", volumeSetterOf(audio.setVolume));
    library.set_function("setMusicVolume", volumeSetterOf(audio.setMusicVolume));
}

} // namespace younity::scripting

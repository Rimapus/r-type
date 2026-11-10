/*
** EPITECH PROJECT, 2026
** j-type/client/src/settings/Settings
** File description:
** Settings header file
*/

#pragma once

#include <filesystem>
#include <optional>
#include <string_view>

#include "input/Bindings.hpp"
#include "settings/Colorblind.hpp"

namespace jtype {

// The highest volume: a volume goes from 0 for silence to it
inline constexpr int MAX_VOLUME = 100;

// The change of a volume at each press of left or right in the settings screen
inline constexpr int VOLUME_STEP = 10;

/**
 * @brief The choices of the player, kept between runs in the settings file.
 */
struct Settings
{
    // From 0 for silence to MAX_VOLUME
    int musicVolume = MAX_VOLUME;
    int effectsVolume = MAX_VOLUME;
    // A line at the bottom of the screen for each sound that tells something, see Captions
    bool captions = false;
    // The ship shoots on its own, without holding the key
    bool autoFire = false;
    // The color blindness the colors of the window are corrected for, see getColorblindMatrix()
    ColorblindMode colorblind = ColorblindMode::Off;
    // The starfield is darkened, see DIMMED_STARFIELD
    bool highContrast = false;
    // The explosions are drawn smaller and dimmer, see getFlashLook()
    bool reduceFlashes = false;
    // The key and the gamepad input of each action of the game
    Bindings bindings = getDefaultBindings();
};

/**
 * @brief Find the settings file in the configuration folder of the user: $XDG_CONFIG_HOME, or ~/.config, on Linux,
 *        ~/Library/Application Support on macOS, %APPDATA% on Windows.
 *
 * @return The path of the file, in the working folder if the system gives no configuration folder.
 */
[[nodiscard]] std::filesystem::path getSettingsPath(void);

/**
 * @brief Read the settings file: one "<name> <value>" per line, # starts a comment.
 *        A setting missing or invalid keeps its default value, so a missing file gives the default settings. A key or a
 *        gamepad input kept for going back is invalid, and one already bound to another action swaps with it.
 *
 * @param path The settings file
 *
 * @return The settings read.
 */
/**
 * @brief Read an on/off argument of a command.
 *
 * @param word The argument, "on" or "off"
 *
 * @return true for "on", false for "off", nothing otherwise.
 */
[[nodiscard]] std::optional<bool> parseSwitch(std::string_view word) noexcept;

/**
 * @brief Read a volume argument of a command.
 *
 * @param word The argument, an integer from 0 to MAX_VOLUME
 *
 * @return The volume, or nothing if the word is not one.
 */
[[nodiscard]] std::optional<int> parseVolume(std::string_view word) noexcept;

[[nodiscard]] Settings loadSettings(const std::filesystem::path &path);

/**
 * @brief Write the settings file, and its folder if it does not exist. The file is written beside it under a
 *        temporary name, then renamed over it, so a crash during the write never leaves it cut.
 *
 * @param path The settings file
 * @param settings The settings to write
 *
 * @return true if the file was written.
 */
[[nodiscard]] bool saveSettings(const std::filesystem::path &path, const Settings &settings);

} // namespace jtype

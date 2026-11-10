/*
** EPITECH PROJECT, 2026
** j-type/client/src/settings/Settings
** File description:
** Settings source file
*/

#include "settings/Settings.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

namespace jtype {

namespace {

const std::filesystem::path SETTINGS_FILE = "settings.txt";

constexpr std::string_view MUSIC_VOLUME_NAME = "music_volume";
constexpr std::string_view EFFECTS_VOLUME_NAME = "effects_volume";
constexpr std::string_view CAPTIONS_NAME = "captions";
constexpr std::string_view AUTO_FIRE_NAME = "auto_fire";
constexpr std::string_view COLORBLIND_NAME = "colorblind";
constexpr std::string_view HIGH_CONTRAST_NAME = "high_contrast";
constexpr std::string_view REDUCE_FLASHES_NAME = "reduce_flashes";

#if defined(_WIN32)
/**
 * @brief Read a folder from an environment variable.
 *
 * @param name The name of the variable
 *
 * @return The folder, empty if the variable is not set.
 */
std::filesystem::path readFolder(const wchar_t *name)
{
    wchar_t *value = nullptr;
    std::size_t size = 0;

    // std::getenv is deprecated by the Windows C library, and narrow: a user name with accents would be lost
    if (_wdupenv_s(&value, &size, name) != 0 || value == nullptr) {
        return {};
    }
    std::filesystem::path folder(value);

    std::free(value);
    return folder;
}
#else
/**
 * @brief Read a folder from an environment variable.
 *
 * @param name The name of the variable
 *
 * @return The folder, empty if the variable is not set.
 */
std::filesystem::path readFolder(const char *name)
{
    const char *value = std::getenv(name);

    return value == nullptr ? std::filesystem::path() : std::filesystem::path(value);
}
#endif

/**
 * @brief Find the folder of the settings of the game, in the configuration folder of the user.
 *
 * @return The folder, empty if the system gives no configuration folder.
 */
std::filesystem::path getSettingsFolder(void)
{
#if defined(_WIN32)
    const std::filesystem::path appData = readFolder(L"APPDATA");

    return appData.empty() ? appData : appData / "J-Type";
#elif defined(__APPLE__)
    const std::filesystem::path home = readFolder("HOME");

    return home.empty() ? home : home / "Library" / "Application Support" / "J-Type";
#else
    const std::filesystem::path config = readFolder("XDG_CONFIG_HOME");
    const std::filesystem::path home = readFolder("HOME");

    // The XDG specification ignores a relative $XDG_CONFIG_HOME
    if (config.is_absolute()) {
        return config / "j-type";
    }
    return home.empty() ? home : home / ".config" / "j-type";
#endif
}

/**
 * @brief Read a value of the file.
 *
 * @param text The value written in the file
 * @param max The highest value allowed
 *
 * @return The value, or nothing if the text is not a whole number from 0 to max.
 */
std::optional<int> parseNumber(std::string_view text, int max)
{
    int number = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), number);

    if (error != std::errc() || end != text.data() + text.size() || number < 0 || number > max) {
        return std::nullopt;
    }
    return number;
}

/**
 * @brief Get the name of the key or of the gamepad input of an action in the file.
 *
 * @param prefix "key" or "pad"
 * @param action The action
 *
 * @return The name, e.g. "key_shoot".
 */
std::string getBindingName(std::string_view prefix, GameAction action)
{
    std::string name = std::format("{}_{}", prefix, getActionName(action));

    std::ranges::transform(name, name.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return name;
}

/**
 * @brief Read the binding of an action, if the line is one.
 *
 * @param name The name of the line
 * @param value Its value
 * @param bindings The bindings to change
 */
void readBinding(std::string_view name, std::string_view value, Bindings &bindings)
{
    for (std::size_t index = 0; index < GAME_ACTION_COUNT; ++index) {
        const auto action = static_cast<GameAction>(index);

        if (name == getBindingName("key", action)) {
            const std::optional<younity::input::Key> key = parseKey(value);

            if (key && !isReserved(*key)) {
                rebindKey(bindings, action, *key);
            }
        } else if (name == getBindingName("pad", action)) {
            const std::optional<GamepadInput> input = parseGamepad(value);

            if (input && !isReserved(*input)) {
                rebindGamepad(bindings, action, *input);
            }
        }
    }
}

} // namespace

std::filesystem::path getSettingsPath(void)
{
    return getSettingsFolder() / SETTINGS_FILE;
}

std::optional<bool> parseSwitch(std::string_view word) noexcept
{
    if (word == "on") {
        return true;
    }
    if (word == "off") {
        return false;
    }
    return std::nullopt;
}

std::optional<int> parseVolume(std::string_view word) noexcept
{
    int volume = 0;
    const std::from_chars_result result = std::from_chars(word.data(), word.data() + word.size(), volume);

    if (result.ec != std::errc{} || result.ptr != word.data() + word.size() || volume < 0 || volume > MAX_VOLUME) {
        return std::nullopt;
    }
    return volume;
}

Settings loadSettings(const std::filesystem::path &path)
{
    Settings settings;
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream words(line);
        std::string name;
        std::string value;

        if (!(words >> name >> value) || name.starts_with('#')) {
            continue;
        }
        // A name of another version of the game is skipped, so an old or a newer file is still read
        if (name == MUSIC_VOLUME_NAME) {
            settings.musicVolume = parseNumber(value, MAX_VOLUME).value_or(settings.musicVolume);
        } else if (name == EFFECTS_VOLUME_NAME) {
            settings.effectsVolume = parseNumber(value, MAX_VOLUME).value_or(settings.effectsVolume);
        } else if (name == CAPTIONS_NAME) {
            // 0 or 1, as saveSettings() writes it
            settings.captions = parseNumber(value, 1).value_or(settings.captions ? 1 : 0) == 1;
        } else if (name == AUTO_FIRE_NAME) {
            settings.autoFire = parseNumber(value, 1).value_or(settings.autoFire ? 1 : 0) == 1;
        } else if (name == COLORBLIND_NAME) {
            settings.colorblind = parseColorblindMode(value).value_or(settings.colorblind);
        } else if (name == HIGH_CONTRAST_NAME) {
            settings.highContrast = parseNumber(value, 1).value_or(settings.highContrast ? 1 : 0) == 1;
        } else if (name == REDUCE_FLASHES_NAME) {
            settings.reduceFlashes = parseNumber(value, 1).value_or(settings.reduceFlashes ? 1 : 0) == 1;
        } else {
            readBinding(name, value, settings.bindings);
        }
    }
    return settings;
}

bool saveSettings(const std::filesystem::path &path, const Settings &settings)
{
    std::error_code error;

    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
    }
    // Written beside the settings, then renamed over them: a crash during the write leaves the old file whole
    std::filesystem::path temporary = path;

    temporary += ".tmp";
    std::ofstream file(temporary);

    file << "# The settings of the J-Type client, written by its settings screen\n"
         << MUSIC_VOLUME_NAME << ' ' << settings.musicVolume << '\n'
         << EFFECTS_VOLUME_NAME << ' ' << settings.effectsVolume << '\n'
         << CAPTIONS_NAME << ' ' << (settings.captions ? 1 : 0) << '\n'
         << AUTO_FIRE_NAME << ' ' << (settings.autoFire ? 1 : 0) << '\n'
         << COLORBLIND_NAME << ' ' << getColorblindName(settings.colorblind) << '\n'
         << HIGH_CONTRAST_NAME << ' ' << (settings.highContrast ? 1 : 0) << '\n'
         << REDUCE_FLASHES_NAME << ' ' << (settings.reduceFlashes ? 1 : 0) << '\n';
    for (std::size_t index = 0; index < GAME_ACTION_COUNT; ++index) {
        const auto action = static_cast<GameAction>(index);
        const Binding &binding = settings.bindings[index];

        file << getBindingName("key", action) << ' ' << getKeyName(binding.key) << '\n'
             << getBindingName("pad", action) << ' ' << getGamepadName(binding.gamepad) << '\n';
    }
    file.close();
    if (file.fail()) {
        std::filesystem::remove(temporary, error);
        return false;
    }
    std::filesystem::rename(temporary, path, error);
    return !error;
}

} // namespace jtype

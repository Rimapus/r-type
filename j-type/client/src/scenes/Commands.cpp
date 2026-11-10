/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Commands
** File description:
** Commands source file
*/

#include "scenes/Commands.hpp"

#include <array>
#include <cctype>
#include <expected>
#include <span>
#include <string>

#include "scenes/MenuScene.hpp"
#include "settings/Colorblind.hpp"
#include "settings/Settings.hpp"

namespace jtype {

namespace {

using Arguments = std::span<const std::string_view>;
using Answer = std::expected<std::string, younity::tools::ConsoleError>;

// The names of the commands of the game, removed together by removeCommands()
constexpr std::array<std::string_view, 8> COMMAND_NAMES{"quit", "menu", "mute", "volume",
                                                        "colorblind", "captions", "autofire", "contrast"};

/**
 * @brief Save the settings changed by a command, like the settings screen does when it is left.
 *
 * @param context What the screens share
 * @param answer The text of the command
 *
 * @return The text, or an error if the settings file cannot be written.
 */
Answer save(const Context &context, std::string answer)
{
    if (!saveSettings(getSettingsPath(), context.settings)) {
        return std::unexpected(younity::tools::ConsoleError{"the settings cannot be saved"});
    }
    return answer;
}

/**
 * @brief Make the handler of an on/off setting.
 *
 * @param context What the screens share
 * @param name The name of the command
 * @param setting The setting it changes
 *
 * @return The handler.
 */
younity::tools::CommandHandler makeSwitch(Context &context, std::string_view name, bool Settings::*setting)
{
    return [&context, name, setting](Arguments arguments) -> Answer {
        const std::optional<bool> on = arguments.size() == 1 ? parseSwitch(arguments.front()) : std::nullopt;

        if (!on) {
            return std::unexpected(younity::tools::ConsoleError{std::string(name) + " takes on or off"});
        }
        context.settings.*setting = *on;
        return save(context, std::string(name) + (*on ? " on" : " off"));
    };
}

} // namespace

void registerCommands(younity::tools::Console &console, Context &context)
{
    console.addCommand("quit", "Close the client", [&context](Arguments) -> Answer {
        context.quit = true;
        return std::string("Bye");
    });
    console.addCommand("menu", "Go back to the main menu", [&context](Arguments) -> Answer {
        context.scenes.changeScene<MenuScene>(context);
        return std::string("Main menu");
    });
    console.addCommand("mute", "Mute or unmute the sound", [&context](Arguments) -> Answer {
        context.audio.toggleMute();
        return std::string("Sound toggled");
    });
    console.addCommand("volume", "Set a volume: volume <music|effects> <0-100>", [&context](Arguments arguments) -> Answer {
        const std::optional<int> volume = arguments.size() == 2 ? parseVolume(arguments[1]) : std::nullopt;

        if (!volume || (arguments[0] != "music" && arguments[0] != "effects")) {
            return std::unexpected(younity::tools::ConsoleError{"volume takes music or effects, then 0 to 100"});
        }
        (arguments[0] == "music" ? context.settings.musicVolume : context.settings.effectsVolume) = *volume;
        context.audio.setVolumes(context.settings);
        return save(context, std::string(arguments[0]) + " volume " + std::to_string(*volume));
    });
    console.addCommand("colorblind", "Correct the colors: colorblind <off|protanopia|deuteranopia|tritanopia>",
                       [&context](Arguments arguments) -> Answer {
                           std::string name(arguments.size() == 1 ? arguments.front() : std::string_view());

                           for (char &character : name) {
                               character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
                           }
                           const std::optional<ColorblindMode> mode = parseColorblindMode(name);

                           if (!mode) {
                               return std::unexpected(younity::tools::ConsoleError{
                                   "colorblind takes off, protanopia, deuteranopia or tritanopia"});
                           }
                           context.settings.colorblind = *mode;
                           applyColorblind(context);
                           return save(context, "colorblind " + name);
                       });
    console.addCommand("captions", "Show the captions of the sounds: captions <on|off>",
                       makeSwitch(context, "captions", &Settings::captions));
    console.addCommand("autofire", "Fire without holding the key: autofire <on|off>",
                       makeSwitch(context, "autofire", &Settings::autoFire));
    console.addCommand("contrast", "Darken the starfield from the next game: contrast <on|off>",
                       makeSwitch(context, "contrast", &Settings::highContrast));
}

void removeCommands(younity::tools::Console &console)
{
    for (const std::string_view name : COMMAND_NAMES) {
        console.removeCommand(name);
    }
}

} // namespace jtype

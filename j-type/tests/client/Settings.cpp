/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Settings
** File description:
** Unit tests of the settings file
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <fstream>
#include <iterator>

#include "input/Bindings.hpp"
#include "settings/Settings.hpp"
#include "younity/input/Key.hpp"

using jtype::loadSettings;
using jtype::parseSwitch;
using jtype::parseVolume;
using jtype::saveSettings;
using jtype::Settings;

namespace {

/**
 * @brief Create an empty folder for the files of a test.
 *
 * @param name The name of the test
 *
 * @return The folder.
 */
std::filesystem::path makeFolder(const char *name)
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "j-type_tests" / name;

    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    return folder;
}

} // namespace

Test(client_settings, gives_the_defaults_without_a_file)
{
    const Settings settings = loadSettings(makeFolder("missing") / "settings.txt");

    cr_assert_eq(settings.musicVolume, jtype::MAX_VOLUME);
    cr_assert_eq(settings.effectsVolume, jtype::MAX_VOLUME);
    cr_assert_not(settings.captions);
    cr_assert_not(settings.autoFire);
    cr_assert(settings.colorblind == jtype::ColorblindMode::Off);
    cr_assert_not(settings.highContrast);
    cr_assert_not(settings.reduceFlashes);
    cr_assert(settings.bindings == jtype::getDefaultBindings());
}

Test(client_settings, reads_the_volumes_saved)
{
    // In a folder that does not exist yet, as on the first run
    const std::filesystem::path path = makeFolder("saved") / "j-type" / "settings.txt";

    cr_assert(saveSettings(path, Settings{.musicVolume = 30, .effectsVolume = 70}));

    const Settings settings = loadSettings(path);

    cr_assert_eq(settings.musicVolume, 30);
    cr_assert_eq(settings.effectsVolume, 70);
}

Test(client_settings, replaces_the_settings_saved_before_without_a_temporary_file)
{
    const std::filesystem::path folder = makeFolder("replaced");
    const std::filesystem::path path = folder / "settings.txt";

    cr_assert(saveSettings(path, Settings{.musicVolume = 30}));
    cr_assert(saveSettings(path, Settings{.musicVolume = 60}));
    cr_assert_eq(loadSettings(path).musicVolume, 60);
    cr_assert_eq(std::distance(std::filesystem::directory_iterator(folder), std::filesystem::directory_iterator()), 1);
}

Test(client_settings, reads_the_bindings_and_the_auto_fire_saved)
{
    const std::filesystem::path path = makeFolder("bindings") / "settings.txt";
    Settings saved;

    jtype::rebindKey(saved.bindings, jtype::GameAction::Shoot, younity::input::Key::X);
    jtype::rebindGamepad(saved.bindings, jtype::GameAction::Mute, jtype::GamepadInput(7U));
    saved.autoFire = true;
    cr_assert(saveSettings(path, saved));

    const Settings settings = loadSettings(path);

    cr_assert(settings.bindings == saved.bindings);
    cr_assert(settings.autoFire);
}

Test(client_settings, refuses_the_keys_kept_for_going_back_and_the_layouts)
{
    const std::filesystem::path path = makeFolder("reserved") / "settings.txt";

    std::ofstream(path) << "key_shoot ESCAPE\npad_shoot B1\nkey_up NOTAKEY\nkey_down F1\nkey_left F2\n";
    cr_assert(loadSettings(path).bindings == jtype::getDefaultBindings());
}

Test(client_settings, reads_the_colorblind_and_high_contrast_saved)
{
    const std::filesystem::path path = makeFolder("contrast") / "settings.txt";

    cr_assert(saveSettings(path, Settings{.colorblind = jtype::ColorblindMode::Tritanopia, .highContrast = true}));

    const Settings settings = loadSettings(path);

    cr_assert(settings.colorblind == jtype::ColorblindMode::Tritanopia);
    cr_assert(settings.highContrast);
}

Test(client_settings, reads_the_reduce_flashes_saved)
{
    const std::filesystem::path path = makeFolder("flashes") / "settings.txt";

    cr_assert(saveSettings(path, Settings{.reduceFlashes = true}));
    cr_assert(loadSettings(path).reduceFlashes);
    cr_assert(saveSettings(path, Settings{}));
    cr_assert_not(loadSettings(path).reduceFlashes);
}

Test(client_settings, reads_the_captions_saved)
{
    const std::filesystem::path path = makeFolder("captions") / "settings.txt";

    cr_assert(saveSettings(path, Settings{.captions = true}));
    cr_assert(loadSettings(path).captions);
}

Test(client_settings, skips_the_comments_and_the_unknown_names)
{
    const std::filesystem::path path = makeFolder("comments") / "settings.txt";

    std::ofstream(path) << "# music_volume 10\n\nlanguage fr\neffects_volume 40\n";

    const Settings settings = loadSettings(path);

    cr_assert_eq(settings.musicVolume, jtype::MAX_VOLUME);
    cr_assert_eq(settings.effectsVolume, 40);
}

Test(client_settings, keeps_the_default_of_an_invalid_volume)
{
    const std::filesystem::path path = makeFolder("invalid") / "settings.txt";

    std::ofstream(path) << "music_volume 150\neffects_volume fifty\ncaptions 2\n";

    const Settings settings = loadSettings(path);

    cr_assert_eq(settings.musicVolume, jtype::MAX_VOLUME);
    cr_assert_eq(settings.effectsVolume, jtype::MAX_VOLUME);
    cr_assert_not(settings.captions);
}

Test(client_settings, cannot_save_where_a_file_is_in_the_way)
{
    const std::filesystem::path folder = makeFolder("blocked");

    std::ofstream(folder / "j-type") << "not a folder\n";
    cr_assert_not(saveSettings(folder / "j-type" / "settings.txt", Settings{}));
}

Test(client_settings, are_kept_in_a_settings_file)
{
    cr_assert(jtype::getSettingsPath().filename() == "settings.txt");
}

Test(client_settings, reads_the_switches_and_the_volumes_of_the_console)
{
    cr_assert(parseSwitch("on") == true);
    cr_assert(parseSwitch("off") == false);
    cr_assert_not(parseSwitch("yes").has_value());
    cr_assert(parseVolume("40") == 40);
    cr_assert_not(parseVolume("101").has_value());
    cr_assert_not(parseVolume("-1").has_value());
    cr_assert_not(parseVolume("4x").has_value());
}

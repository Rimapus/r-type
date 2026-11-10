/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/SettingsScene
** File description:
** SettingsScene source file
*/

#include "scenes/SettingsScene.hpp"

#include <algorithm>
#include <format>
#include <string>
#include <string_view>

#include "render/BitmapFont.hpp"
#include "scenes/ControlsScene.hpp"
#include "scenes/Menu.hpp"
#include "scenes/MenuScene.hpp"
#include "settings/Colorblind.hpp"
#include "settings/Settings.hpp"

namespace jtype {

namespace {

// The lines of the screen, from the top
constexpr std::size_t MUSIC_LINE = 0;
constexpr std::size_t EFFECTS_LINE = 1;
constexpr std::size_t CAPTIONS_LINE = 2;
constexpr std::size_t AUTO_FIRE_LINE = 3;
constexpr std::size_t COLORBLIND_LINE = 4;
constexpr std::size_t HIGH_CONTRAST_LINE = 5;
constexpr std::size_t REDUCE_FLASHES_LINE = 6;
constexpr std::size_t CONTROLS_LINE = 7;
constexpr std::size_t BACK_LINE = 8;

constexpr float TITLE_TOP = 16.0F;
constexpr float TITLE_SIZE = 3.0F;
constexpr float LINES_TOP = 56.0F;

/**
 * @brief Write the line of a volume.
 *
 * @param name What the volume is of
 * @param volume The volume, from 0 to MAX_VOLUME
 *
 * @return The text of the line.
 */
std::string formatVolume(std::string_view name, int volume)
{
    return std::format("{} {}%", name, volume);
}

/**
 * @brief Write the line of the color blindness the colors are corrected for.
 *
 * @param mode The color blindness
 *
 * @return The text of the line, e.g. "COLORBLIND DEUTERANOPIA".
 */
std::string formatColorblind(ColorblindMode mode)
{
    return std::format("COLORBLIND {}", getColorblindName(mode));
}

/**
 * @brief Write the line of an option switched on or off.
 *
 * @param name The option
 * @param on true if it is on
 *
 * @return The text of the line.
 */
std::string formatSwitch(std::string_view name, bool on)
{
    return std::format("{} {}", name, on ? "ON" : "OFF");
}

} // namespace

SettingsScene::SettingsScene(Context &context)
    : MenuScreen(context,
                 {formatVolume("MUSIC", context.settings.musicVolume),
                  formatVolume("EFFECTS", context.settings.effectsVolume), formatSwitch("CAPTIONS", context.settings.captions),
                  formatSwitch("AUTO FIRE", context.settings.autoFire), formatColorblind(context.settings.colorblind),
                  formatSwitch("HIGH CONTRAST", context.settings.highContrast),
                  formatSwitch("REDUCE FLASHES", context.settings.reduceFlashes), "CONTROLS", "BACK"},
                 LINES_TOP)
{
    this->write(centerText("SETTINGS", TITLE_TOP, FontColor::Amber, TITLE_SIZE));
}

void SettingsScene::confirm(std::size_t line)
{
    // Confirm switches an option too, like left and right
    if (line == CAPTIONS_LINE || line == AUTO_FIRE_LINE || line == COLORBLIND_LINE || line == HIGH_CONTRAST_LINE ||
        line == REDUCE_FLASHES_LINE) {
        this->adjust(line, 1);
    } else if (line == CONTROLS_LINE) {
        this->getContext().scenes.changeScene<ControlsScene>(this->getContext());
    } else if (line == BACK_LINE) {
        this->back();
    }
}

void SettingsScene::adjust(std::size_t line, int delta)
{
    Context &context = this->getContext();

    if (line == MUSIC_LINE) {
        context.settings.musicVolume = std::clamp(context.settings.musicVolume + delta * VOLUME_STEP, 0, MAX_VOLUME);
        this->setLine(line, formatVolume("MUSIC", context.settings.musicVolume));
    } else if (line == EFFECTS_LINE) {
        context.settings.effectsVolume =
            std::clamp(context.settings.effectsVolume + delta * VOLUME_STEP, 0, MAX_VOLUME);
        this->setLine(line, formatVolume("EFFECTS", context.settings.effectsVolume));
    } else if (line == CAPTIONS_LINE) {
        context.settings.captions = !context.settings.captions;
        this->setLine(line, formatSwitch("CAPTIONS", context.settings.captions));
    } else if (line == AUTO_FIRE_LINE) {
        context.settings.autoFire = !context.settings.autoFire;
        this->setLine(line, formatSwitch("AUTO FIRE", context.settings.autoFire));
    } else if (line == COLORBLIND_LINE) {
        const auto mode = static_cast<int>(context.settings.colorblind);
        const auto count = static_cast<int>(COLORBLIND_MODE_COUNT);

        context.settings.colorblind = static_cast<ColorblindMode>((mode + delta % count + count) % count);
        applyColorblind(context);
        this->setLine(line, formatColorblind(context.settings.colorblind));
    } else if (line == HIGH_CONTRAST_LINE) {
        context.settings.highContrast = !context.settings.highContrast;
        this->applyContrast();
        this->setLine(line, formatSwitch("HIGH CONTRAST", context.settings.highContrast));
    } else if (line == REDUCE_FLASHES_LINE) {
        context.settings.reduceFlashes = !context.settings.reduceFlashes;
        this->setLine(line, formatSwitch("REDUCE FLASHES", context.settings.reduceFlashes));
    }
    context.audio.setVolumes(context.settings);
}

void SettingsScene::back(void)
{
    Context &context = this->getContext();

    // The settings still apply to this run when they cannot be saved: the player is told they will not last
    if (saveSettings(getSettingsPath(), context.settings)) {
        context.scenes.changeScene<MenuScene>(context);
    } else {
        context.scenes.changeScene<MenuScene>(context, "the settings could not be saved");
    }
}

} // namespace jtype

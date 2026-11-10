/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/ControlsScene
** File description:
** ControlsScene source file
*/

#include "scenes/ControlsScene.hpp"

#include <format>
#include <string>
#include <vector>

#include "render/BitmapFont.hpp"
#include "scenes/Menu.hpp"
#include "scenes/SettingsScene.hpp"
#include "younity/input/Key.hpp"

namespace jtype {

namespace {

// The lines of the screen, from the top: one per action of the game, then these
constexpr std::size_t RESET_LINE = GAME_ACTION_COUNT;

constexpr float TITLE_TOP = 12.0F;
constexpr float TITLE_SIZE = 2.0F;
constexpr float LINES_TOP = 40.0F;
constexpr float HINT_TOP = 250.0F;

/**
 * @brief Write the lines of the screen.
 *
 * @param bindings The bindings of the player
 *
 * @return The lines, from the top.
 */
std::vector<std::string> getLines(const Bindings &bindings)
{
    std::vector<std::string> lines;

    for (std::size_t action = 0; action < GAME_ACTION_COUNT; ++action) {
        lines.push_back(formatBinding(static_cast<GameAction>(action), bindings[action]));
    }
    lines.emplace_back("RESET");
    lines.emplace_back("BACK");
    return lines;
}

} // namespace

ControlsScene::ControlsScene(Context &context)
    : MenuScreen(context, getLines(context.settings.bindings), LINES_TOP)
{
    this->write(centerText("CONTROLS", TITLE_TOP, FontColor::Amber, TITLE_SIZE));
    this->write(centerText("ENTER TO CHANGE, ESCAPE TO CANCEL", HINT_TOP, FontColor::White, 1.0F));
}

bool ControlsScene::interceptInput(void)
{
    if (!this->_capturing) {
        return false;
    }
    Context &context = this->getContext();

    // Read before the key and the button: Escape and the back button cancel, so they are never bound
    if (context.controls.isMenuPressed(MenuAction::Back)) {
        this->finishCapture();
    } else if (const std::optional<younity::input::Key> key = context.controls.getKeyJustPressed(); key && !isReserved(*key)) {
        rebindKey(context.settings.bindings, *this->_capturing, *key);
        this->finishCapture();
    } else if (const std::optional<GamepadInput> input = context.controls.getGamepadJustPressed()) {
        rebindGamepad(context.settings.bindings, *this->_capturing, *input);
        this->finishCapture();
    }
    return true;
}

void ControlsScene::confirm(std::size_t line)
{
    Context &context = this->getContext();

    if (line < GAME_ACTION_COUNT) {
        this->_capturing = static_cast<GameAction>(line);
        this->setLine(line, std::format("{}: PRESS A KEY OR BUTTON", getActionName(*this->_capturing)));
    } else if (line == RESET_LINE) {
        context.settings.bindings = getDefaultBindings();
        context.controls.apply(context.settings.bindings);
        this->drawBindings();
    } else {
        this->back();
    }
}

void ControlsScene::back(void)
{
    this->getContext().scenes.changeScene<SettingsScene>(this->getContext());
}

void ControlsScene::finishCapture(void)
{
    Context &context = this->getContext();

    this->_capturing.reset();
    context.controls.apply(context.settings.bindings);
    this->drawBindings();
}

void ControlsScene::drawBindings(void)
{
    const Bindings &bindings = this->getContext().settings.bindings;

    for (std::size_t action = 0; action < GAME_ACTION_COUNT; ++action) {
        this->setLine(action, formatBinding(static_cast<GameAction>(action), bindings[action]));
    }
}

} // namespace jtype

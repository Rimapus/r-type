/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/MenuScene
** File description:
** MenuScene source file
*/

#include "scenes/MenuScene.hpp"

#include <array>

#include "render/BitmapFont.hpp"
#include "scenes/GameScene.hpp"
#include "scenes/HelpScene.hpp"
#include "scenes/Menu.hpp"
#include "scenes/SettingsScene.hpp"
#include "system/JClimb.hpp"

namespace jtype {

namespace {

// The lines of the menu, from the top
constexpr std::size_t PLAY_LINE = 0;
constexpr std::size_t HELP_LINE = 1;
constexpr std::size_t SETTINGS_LINE = 2;

constexpr float TITLE_TOP = 40.0F;
constexpr float TITLE_SIZE = 4.0F;
constexpr float MESSAGE_TOP = 100.0F;
constexpr float LINES_TOP = 130.0F;

constexpr std::array<MenuAction, 6> MENU_ACTIONS{
    MenuAction::Up,
    MenuAction::Down,
    MenuAction::Left,
    MenuAction::Right,
    MenuAction::Confirm,
    MenuAction::Back,
};

} // namespace

MenuScene::MenuScene(Context &context, const std::string &message)
    : MenuScreen(context, {"PLAY", "HELP", "SETTINGS", "QUIT"}, LINES_TOP)
{
    // The music stopped at a game over starts again from the beginning; a new game starts it itself
    context.audio.setGameOver(false);
    this->write(centerText("J-TYPE", TITLE_TOP, FontColor::Amber, TITLE_SIZE));
    if (!message.empty()) {
        this->write(centerText(message, MESSAGE_TOP, FontColor::White, 1.0F));
    }
}

void MenuScene::confirm(std::size_t line)
{
    Context &context = this->getContext();

    if (line == PLAY_LINE) {
        context.scenes.changeScene<GameScene>(context);
    } else if (line == HELP_LINE) {
        context.scenes.changeScene<HelpScene>(context);
    } else if (line == SETTINGS_LINE) {
        context.scenes.changeScene<SettingsScene>(context);
    } else {
        context.quit = true;
    }
}

bool MenuScene::interceptInput(void)
{
    Context &context = this->getContext();

    for (const MenuAction action : MENU_ACTIONS) {
        if (context.controls.isMenuPressed(action) && this->_konami.enter(action)) {
            startJClimb(context.jclimb);
        }
    }
    return false;
}

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/HelpScene
** File description:
** HelpScene source file
*/

#include "scenes/HelpScene.hpp"

#include <string>

#include "render/BitmapFont.hpp"
#include "scenes/Help.hpp"
#include "scenes/Menu.hpp"
#include "scenes/MenuScene.hpp"

namespace jtype {

namespace {

constexpr float TITLE_TOP = 12.0F;
constexpr float TITLE_SIZE = 2.0F;
constexpr float TEXT_TOP = 40.0F;
// From the top of a line of help to the top of the next one, in pixels of the game area
constexpr float TEXT_SPACING = 11.0F;
constexpr float BACK_TOP = 230.0F;

} // namespace

HelpScene::HelpScene(Context &context)
    : MenuScreen(context, {"BACK"}, BACK_TOP)
{
    float top = TEXT_TOP;

    this->write(centerText("HELP", TITLE_TOP, FontColor::Amber, TITLE_SIZE));
    for (const std::string &line : getHelpLines(context.settings.bindings)) {
        this->write(centerText(line, top, FontColor::White, 1.0F));
        top += TEXT_SPACING;
    }
}

void HelpScene::confirm(std::size_t /* line */)
{
    this->back();
}

void HelpScene::back(void)
{
    this->getContext().scenes.changeScene<MenuScene>(this->getContext());
}

} // namespace jtype

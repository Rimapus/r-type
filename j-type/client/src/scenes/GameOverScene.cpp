/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/GameOverScene
** File description:
** GameOverScene source file
*/

#include "scenes/GameOverScene.hpp"

#include "scenes/GameScene.hpp"
#include "scenes/MenuScene.hpp"

namespace jtype {

namespace {

// The lines of the screen, from the top
constexpr std::size_t PLAY_AGAIN_LINE = 0;

// Under the GAME OVER of the HUD, in the middle of the screen
constexpr float LINES_TOP = 170.0F;

} // namespace

GameOverScene::GameOverScene(Context &context, const ScoreboardMessage &scoreboard, std::uint8_t slot)
    : MenuScreen(context, {"PLAY AGAIN", "MENU"}, LINES_TOP),
      _hud(this->getRegistry(), context.assets)
{
    this->_hud.show(scoreboard, slot);
}

void GameOverScene::confirm(std::size_t line)
{
    Context &context = this->getContext();

    if (line == PLAY_AGAIN_LINE) {
        context.scenes.changeScene<GameScene>(context);
    } else {
        context.scenes.changeScene<MenuScene>(context);
    }
}

void GameOverScene::back(void)
{
    this->getContext().scenes.changeScene<MenuScene>(this->getContext());
}

} // namespace jtype

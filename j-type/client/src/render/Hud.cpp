/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Hud
** File description:
** Hud source file
*/

#include "render/Hud.hpp"

#include <format>
#include <string_view>

#include "render/HudLayout.hpp"

namespace jtype {

namespace {

// The first image of the life icons
constexpr younity::Recti LIFE_FRAME{0, 0, 12, 12};

// The images of a full and of an empty cell of the health bar of the boss
constexpr std::string_view CELL_TEXTURE = "sprites/16bit/ui/hp_pip.png";
constexpr younity::Recti FULL_CELL_FRAME{0, 0, 10, 10};
constexpr younity::Recti EMPTY_CELL_FRAME{0, 10, 10, 10};

} // namespace

Hud::Hud(younity::ecs::Registry &registry, Assets &assets)
    : _assets(assets),
      _overlay(registry, assets),
      _bossOverlay(registry, assets)
{
}

void Hud::show(const ScoreboardMessage &scoreboard, std::optional<std::uint8_t> local)
{
    // The server sends the scoreboard at each step, but it changes only with a score, a life or the game over
    if (scoreboard == this->_shown && local == this->_local) {
        return;
    }
    this->_shown = scoreboard;
    this->_local = local;
    this->_overlay.clear();

    const HudLayout layout = layoutHud(scoreboard, local);

    for (const HudText &text : layout.texts) {
        this->_overlay.write(text);
    }
    for (const HudLife &life : layout.lives) {
        this->_overlay.addSprite(this->_assets.loadTexture(std::format("sprites/16bit/ui/life_icon_p{}.png", life.slot + 1)), LIFE_FRAME,
                                 life.position, 1.0F);
    }
}

void Hud::showBoss(std::optional<std::pair<int, int>> health)
{
    if (health == this->_bossHealth) {
        return;
    }
    this->_bossHealth = health;
    this->_bossOverlay.clear();
    if (!health) {
        return;
    }
    const BossBarLayout layout = layoutBossBar(health->first, health->second);

    this->_bossOverlay.write(layout.title);
    for (const HudCell &cell : layout.cells) {
        this->_bossOverlay.addSprite(this->_assets.loadTexture(CELL_TEXTURE), cell.isFull ? FULL_CELL_FRAME : EMPTY_CELL_FRAME,
                                     cell.position, 1.0F);
    }
}

} // namespace jtype

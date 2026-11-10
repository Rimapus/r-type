/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/HudLayout
** File description:
** HudLayout source file
*/

#include "render/HudLayout.hpp"

#include <algorithm>
#include <format>
#include <string_view>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"

namespace jtype {

namespace {

// The space between the HUD and the edges of the game area, in pixels of the game area
constexpr float HUD_MARGIN = 4.0F;

// The width of the HUD given to each slot
constexpr float SLOT_WIDTH = WORLD_WIDTH / static_cast<float>(MAX_PLAYERS);

// The top of the lives, under the line of the name and the score
constexpr float LIVES_TOP = HUD_MARGIN + static_cast<float>(GLYPH_SIZE) + 2.0F;

// The width of a life icon and the space after it
constexpr float LIFE_ADVANCE = 13.0F;

// The name of the weapon is 8 pixels high, the life icons 12: centered on them
constexpr float WEAPON_DROP = 2.0F;

/**
 * @brief Get the name of a weapon, as the HUD writes it.
 *
 * @param weapon The weapon
 *
 * @return Its name, in capitals.
 */
std::string_view getWeaponName(Weapon weapon)
{
    switch (weapon) {
    case Weapon::Spread:
        return "SPREAD";
    case Weapon::Plasma:
        break;
    }
    return "PLASMA";
}

// The health bar of the boss, under the lives: its cells are 10 pixels wide and 9 apart, so they touch
constexpr int BOSS_BAR_CELLS = 20;
constexpr float BOSS_BAR_TOP = LIVES_TOP + 16.0F;
constexpr float BOSS_CELL_SIZE = 10.0F;
constexpr float BOSS_CELL_ADVANCE = 9.0F;
constexpr std::string_view BOSS_TITLE = "BOSS";

// GAME OVER or VICTORY is written bigger than the rest of the HUD, as the screen of the end of the game
constexpr float GAME_OVER_SIZE = 3.0F;
constexpr std::string_view GAME_OVER_TEXT = "GAME OVER";
constexpr std::string_view VICTORY_TEXT = "VICTORY";

} // namespace

HudLayout layoutHud(const ScoreboardMessage &scoreboard, std::optional<std::uint8_t> local)
{
    HudLayout layout;

    for (std::size_t slot = 0; slot < MAX_PLAYERS; ++slot) {
        const PlayerStatus &player = scoreboard.players[slot];
        const float left = HUD_MARGIN + SLOT_WIDTH * static_cast<float>(slot);
        const FontColor color = local && *local == slot ? FontColor::Amber : FontColor::White;
        const younity::Vector2f name{left, HUD_MARGIN};
        const younity::Vector2f below{left, LIVES_TOP};

        if (player.state == SlotState::Free) {
            continue;
        }
        layout.texts.push_back(HudText{std::format("P{} {:06}", slot + 1, player.score), name, color, 1.0F});
        if (player.state == SlotState::Out) {
            layout.texts.push_back(HudText{"OUT", below, color, 1.0F});
        } else if (player.state == SlotState::Left) {
            layout.texts.push_back(HudText{"LEFT", below, color, 1.0F});
        } else {
            for (std::uint8_t life = 0; life < player.lives; ++life) {
                const younity::Vector2f icon{below.x + LIFE_ADVANCE * static_cast<float>(life), below.y};

                layout.lives.push_back(HudLife{slot, icon});
            }
            // After the room of the most lives, so it stays in place when a life is won or lost
            const younity::Vector2f weapon{below.x + (LIFE_ADVANCE * static_cast<float>(MAX_LIVES)), below.y + WEAPON_DROP};

            layout.texts.push_back(HudText{std::string(getWeaponName(player.weapon)), weapon, color, 1.0F});
        }
    }
    if (scoreboard.gameOver) {
        const std::string_view text = scoreboard.victory ? VICTORY_TEXT : GAME_OVER_TEXT;
        const float width = static_cast<float>(text.size()) * GLYPH_ADVANCE * GAME_OVER_SIZE;
        const float height = static_cast<float>(GLYPH_SIZE) * GAME_OVER_SIZE;
        const younity::Vector2f topLeft{(WORLD_WIDTH - width) / 2.0F, (WORLD_HEIGHT - height) / 2.0F};

        layout.texts.push_back(HudText{std::string(text), topLeft, FontColor::Amber, GAME_OVER_SIZE});
    }
    return layout;
}

std::optional<int> getBossHealth(const SnapshotMessage &snapshot)
{
    for (const EntityState &entity : snapshot.entities) {
        if (entity.type == EntityType::IncubatorEye || entity.type == EntityType::IncubatorEyeAngry) {
            return entity.health;
        }
    }
    return std::nullopt;
}

BossBarLayout layoutBossBar(int health, int max)
{
    const float titleWidth = (static_cast<float>(BOSS_TITLE.size()) * GLYPH_ADVANCE) + GLYPH_ADVANCE;
    const float cellsWidth = (static_cast<float>(BOSS_BAR_CELLS - 1) * BOSS_CELL_ADVANCE) + BOSS_CELL_SIZE;
    const float left = (WORLD_WIDTH - titleWidth - cellsWidth) / 2.0F;
    // Rounded up: a boss with health left keeps a full cell
    const int full = ((std::clamp(health, 0, max) * BOSS_BAR_CELLS) + max - 1) / max;
    // The text is 8 pixels high, the cells 10: centered on them
    BossBarLayout layout{
        HudText{std::string(BOSS_TITLE), {left, BOSS_BAR_TOP + 1.0F}, FontColor::Amber, 1.0F},
        {}
    };

    for (int cell = 0; cell < BOSS_BAR_CELLS; ++cell) {
        const younity::Vector2f position{left + titleWidth + (static_cast<float>(cell) * BOSS_CELL_ADVANCE), BOSS_BAR_TOP};

        layout.cells.push_back(HudCell{position, cell < full});
    }
    return layout;
}

} // namespace jtype

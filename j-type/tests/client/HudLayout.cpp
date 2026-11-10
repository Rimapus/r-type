/*
** EPITECH PROJECT, 2026
** j-type/tests/client/HudLayout
** File description:
** Unit tests of the layout of the HUD
*/

#include <criterion/criterion.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Messages.hpp"
#include "render/HudLayout.hpp"

using jtype::BossBarLayout;
using jtype::EntityState;
using jtype::EntityType;
using jtype::FontColor;
using jtype::HudLayout;
using jtype::HudText;
using jtype::layoutHud;
using jtype::PlayerStatus;
using jtype::ScoreboardMessage;
using jtype::SlotState;
using jtype::SnapshotMessage;

Test(client_hud_layout, shows_nothing_before_a_player_joins)
{
    const HudLayout layout = layoutHud(ScoreboardMessage{}, std::nullopt);

    cr_assert(layout.texts.empty());
    cr_assert(layout.lives.empty());
}

Test(client_hud_layout, shows_the_score_and_the_lives_of_a_player)
{
    ScoreboardMessage scoreboard;

    scoreboard.players[1] = PlayerStatus{SlotState::Playing, 1200, 3};

    const HudLayout layout = layoutHud(scoreboard, std::nullopt);

    cr_assert_eq(layout.texts.size(), 2U);
    cr_assert_str_eq(layout.texts[0].string.c_str(), "P2 001200");
    cr_assert_str_eq(layout.texts[1].string.c_str(), "PLASMA");
    // The second quarter of the top of the screen
    cr_assert_geq(layout.texts[0].position.x, jtype::WORLD_WIDTH / 4.0F);
    cr_assert_lt(layout.texts[0].position.x, jtype::WORLD_WIDTH / 2.0F);
    cr_assert_eq(layout.lives.size(), 3U);
    for (const jtype::HudLife &life : layout.lives) {
        cr_assert_eq(life.slot, 1U);
    }
}

Test(client_hud_layout, shows_out_for_a_player_out_of_lives)
{
    ScoreboardMessage scoreboard;

    scoreboard.players[0] = PlayerStatus{SlotState::Out, 300, 0};

    const HudLayout layout = layoutHud(scoreboard, std::nullopt);

    cr_assert_eq(layout.texts.size(), 2U);
    cr_assert_str_eq(layout.texts[0].string.c_str(), "P1 000300");
    cr_assert_str_eq(layout.texts[1].string.c_str(), "OUT");
    cr_assert(layout.lives.empty());
}

Test(client_hud_layout, shows_left_for_a_player_who_quit)
{
    ScoreboardMessage scoreboard;

    scoreboard.players[2] = PlayerStatus{SlotState::Left, 800, 0};

    const HudLayout layout = layoutHud(scoreboard, std::nullopt);

    cr_assert_eq(layout.texts.size(), 2U);
    cr_assert_str_eq(layout.texts[0].string.c_str(), "P3 000800");
    cr_assert_str_eq(layout.texts[1].string.c_str(), "LEFT");
    cr_assert(layout.lives.empty());
}

Test(client_hud_layout, shows_this_client_in_amber)
{
    ScoreboardMessage scoreboard;

    scoreboard.players[0] = PlayerStatus{SlotState::Playing, 0, 3};
    scoreboard.players[1] = PlayerStatus{SlotState::Playing, 0, 3};

    const HudLayout layout = layoutHud(scoreboard, std::uint8_t{1});

    // The name and the weapon of each player
    cr_assert(layout.texts[0].color == FontColor::White);
    cr_assert(layout.texts[1].color == FontColor::White);
    cr_assert(layout.texts[2].color == FontColor::Amber);
    cr_assert(layout.texts[3].color == FontColor::Amber);
}

Test(client_hud_layout, shows_the_weapon_after_the_room_of_the_lives)
{
    ScoreboardMessage scoreboard;

    scoreboard.players[0] = PlayerStatus{SlotState::Playing, 0, 1, jtype::Weapon::Spread};

    const HudLayout layout = layoutHud(scoreboard, std::nullopt);

    cr_assert_str_eq(layout.texts[1].string.c_str(), "SPREAD");
    // The room of MAX_LIVES icons, so a life won or lost does not move it
    cr_assert_gt(layout.texts[1].position.x, layout.lives[0].position.x + (12.0F * static_cast<float>(jtype::MAX_LIVES)));
    cr_assert_lt(layout.texts[1].position.x + (6.0F * jtype::GLYPH_ADVANCE), jtype::WORLD_WIDTH / 4.0F);
}

Test(client_hud_layout, shows_game_over_big_in_the_middle)
{
    ScoreboardMessage scoreboard;

    scoreboard.players[0] = PlayerStatus{SlotState::Out, 4500, 0};
    scoreboard.gameOver = true;

    const HudLayout layout = layoutHud(scoreboard, std::uint8_t{0});
    const HudText &gameOver = layout.texts.back();
    const float width = 9.0F * jtype::GLYPH_ADVANCE * gameOver.size;

    cr_assert_str_eq(gameOver.string.c_str(), "GAME OVER");
    cr_assert_gt(gameOver.size, 1.0F);
    cr_assert_float_eq(gameOver.position.x + width / 2.0F, jtype::WORLD_WIDTH / 2.0F, 0.001F);
    // The final score stays at the top
    cr_assert_str_eq(layout.texts[0].string.c_str(), "P1 004500");
}

Test(client_hud_layout, shows_victory_once_the_boss_is_beaten)
{
    ScoreboardMessage scoreboard;

    scoreboard.players[0] = PlayerStatus{SlotState::Playing, 9000, 2};
    scoreboard.gameOver = true;
    scoreboard.victory = true;

    const HudLayout layout = layoutHud(scoreboard, std::uint8_t{0});
    const HudText &victory = layout.texts.back();
    const float width = 7.0F * jtype::GLYPH_ADVANCE * victory.size;

    cr_assert_str_eq(victory.string.c_str(), "VICTORY");
    cr_assert_float_eq(victory.position.x + width / 2.0F, jtype::WORLD_WIDTH / 2.0F, 0.001F);
}

Test(client_hud_layout, boss_health_is_the_one_of_its_eye)
{
    SnapshotMessage snapshot;

    cr_assert_not(jtype::getBossHealth(snapshot).has_value());
    snapshot.entities = std::vector<EntityState>{
        EntityState{1, EntityType::IncubatorBody,     0.0F, 0.0F, 0 },
        EntityState{2, EntityType::IncubatorEyeAngry, 0.0F, 0.0F, 25}
    };
    cr_assert_eq(jtype::getBossHealth(snapshot), 25);
}

Test(client_hud_layout, boss_bar_fills_with_the_health_left)
{
    const auto countFull = [](const BossBarLayout &bar) {
        std::size_t full = 0;

        for (const jtype::HudCell &cell : bar.cells) {
            full += cell.isFull ? 1 : 0;
        }
        return full;
    };
    const BossBarLayout half = jtype::layoutBossBar(30, 60);

    cr_assert_str_eq(half.title.string.c_str(), "BOSS");
    cr_assert_eq(half.cells.size(), 20U);
    cr_assert_eq(countFull(half), 10U);
    cr_assert(half.cells.front().isFull);
    cr_assert_not(half.cells.back().isFull);
    cr_assert_eq(countFull(jtype::layoutBossBar(60, 60)), 20U);
    // A boss with one health point left still has a full cell
    cr_assert_eq(countFull(jtype::layoutBossBar(1, 60)), 1U);
    cr_assert_eq(countFull(jtype::layoutBossBar(0, 60)), 0U);
}

Test(client_hud_layout, boss_bar_is_in_the_middle_under_the_hud)
{
    const BossBarLayout bar = jtype::layoutBossBar(60, 60);
    const float right = bar.cells.back().position.x + 10.0F;

    cr_assert_float_eq((bar.title.position.x + right) / 2.0F, jtype::WORLD_WIDTH / 2.0F, 0.001F);
    cr_assert_gt(bar.title.position.y, 20.0F);
}

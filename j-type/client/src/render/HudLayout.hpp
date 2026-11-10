/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/HudLayout
** File description:
** HudLayout header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "jtype/common/Messages.hpp"
#include "render/BitmapFont.hpp"
#include "younity/core/Color.hpp"
#include "younity/core/Vector2.hpp"

namespace jtype {

/**
 * @brief A text of the HUD, written with the 8x8 font.
 */
struct HudText
{
    std::string string;
    // The top left corner of the first character, in pixels of the game area
    younity::Vector2f position;
    FontColor color;
    // The times the font is drawn bigger than the game area
    float size;
    // Multiplied with the font: white keeps its color, another one tints it, e.g. in the color of a player, and the alpha
    // fades it out
    younity::Color tint{};
};

/**
 * @brief A life left to a player, drawn with the icon of its slot.
 */
struct HudLife
{
    std::size_t slot;
    // The top left corner of the icon, in pixels of the game area
    younity::Vector2f position;
};

/**
 * @brief Everything the HUD draws for one scoreboard.
 */
struct HudLayout
{
    std::vector<HudText> texts;
    std::vector<HudLife> lives;
};

/**
 * @brief Lay out the HUD: along the top, the name, the score, the lives and the weapon of each player who joined,
 *        OUT under a player out of lives, LEFT under a player who quit, and GAME OVER in the middle once the game
 *        is over, or VICTORY once the boss is beaten.
 *        The texts of this client are amber, the ones of the other players are white.
 *
 * @param scoreboard The last scoreboard received
 * @param local The slot of this client, or nothing while it has not joined
 *
 * @return What to draw.
 */
[[nodiscard]] HudLayout layoutHud(const ScoreboardMessage &scoreboard, std::optional<std::uint8_t> local);

/**
 * @brief A cell of the health bar of the boss, drawn with the full or the empty image of the health cells.
 */
struct HudCell
{
    // The top left corner of the cell, in pixels of the game area
    younity::Vector2f position;
    bool isFull;
};

/**
 * @brief Everything the health bar of the boss draws.
 */
struct BossBarLayout
{
    HudText title;
    std::vector<HudCell> cells;
};

/**
 * @brief Find the health of the eye of the boss in a snapshot.
 *
 * @param snapshot The world drawn
 *
 * @return Its health points, or nothing when the boss is not there.
 */
[[nodiscard]] std::optional<int> getBossHealth(const SnapshotMessage &snapshot);

/**
 * @brief Lay out the health bar of the boss, in the middle under the HUD: BOSS, then BOSS_BAR_CELLS cells, the full
 *        ones first, as many as the share of health left, rounded up so the bar is empty only at 0.
 *
 * @param health The health points left
 * @param max The health points of the boss when it came, above 0
 *
 * @return What to draw.
 */
[[nodiscard]] BossBarLayout layoutBossBar(int health, int max);

} // namespace jtype

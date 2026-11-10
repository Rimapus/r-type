/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Hud
** File description:
** Hud header file
*/

#pragma once

#include <cstdint>
#include <optional>
#include <utility>

#include "jtype/common/Messages.hpp"
#include "render/Assets.hpp"
#include "render/Overlay.hpp"
#include "younity/ecs/Registry.hpp"

namespace jtype {

/**
 * @brief The interface drawn over the game: the score and the lives of each player, and the game over.
 */
class Hud
{
public:
    /**
     * @brief Create an empty interface.
     *
     * @param registry The registry holding the sprites, it must outlive the interface
     * @param assets The textures of the font and the icons, they must outlive the interface
     */
    Hud(younity::ecs::Registry &registry, Assets &assets);

    /**
     * @brief Show a scoreboard, see layoutHud(). The sprites are created again only when it changes.
     *
     * @param scoreboard The last scoreboard received
     * @param local The slot of this client, or nothing while it has not joined
     */
    void show(const ScoreboardMessage &scoreboard, std::optional<std::uint8_t> local);

    /**
     * @brief Show the health bar of the boss, see layoutBossBar(), or hide it. The sprites are created again only when
     *        the health changes.
     *
     * @param health The health points left and the ones the boss came with, or nothing to hide the bar
     */
    void showBoss(std::optional<std::pair<int, int>> health);

private:
    Assets &_assets;
    Overlay _overlay;
    ScoreboardMessage _shown;
    std::optional<std::uint8_t> _local;
    // The health bar of the boss, apart from the scoreboard: it changes with the snapshots
    Overlay _bossOverlay;
    std::optional<std::pair<int, int>> _bossHealth;
};

} // namespace jtype

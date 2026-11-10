/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/GameOverScene
** File description:
** GameOverScene header file
*/

#pragma once

#include <cstddef>
#include <cstdint>

#include "jtype/common/Messages.hpp"
#include "render/Hud.hpp"
#include "scenes/Context.hpp"
#include "scenes/MenuScreen.hpp"

namespace jtype {

/**
 * @brief The end of a game: the last scoreboard and GAME OVER, or VICTORY once the boss is beaten, then PLAY AGAIN
 *        joins the server again and MENU goes back to the main menu.
 */
class GameOverScene : public MenuScreen
{
public:
    /**
     * @brief Create the screen with PLAY AGAIN selected.
     *
     * @param context What the scenes share
     * @param scoreboard The last scoreboard of the game
     * @param slot The slot this client played in
     */
    GameOverScene(Context &context, const ScoreboardMessage &scoreboard, std::uint8_t slot);

private:
    void confirm(std::size_t line) override;
    void back(void) override;

    Hud _hud;
};

} // namespace jtype

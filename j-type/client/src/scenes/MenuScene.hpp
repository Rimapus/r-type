/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/MenuScene
** File description:
** MenuScene header file
*/

#pragma once

#include <cstddef>
#include <string>

#include "scenes/Context.hpp"
#include "scenes/KonamiCode.hpp"
#include "scenes/MenuScreen.hpp"

namespace jtype {

/**
 * @brief The main menu, the first screen: PLAY joins the server, HELP recalls the goal and the controls, SETTINGS
 *        opens the settings, QUIT closes the game.
 */
class MenuScene : public MenuScreen
{
public:
    /**
     * @brief Create the main menu with PLAY selected.
     *
     * @param context What the scenes share
     * @param message Why the last game stopped, e.g. the server is full, written under the title; empty for none
     */
    explicit MenuScene(Context &context, const std::string &message = "");

private:
    void confirm(std::size_t line) override;

    /**
     * @brief Start J-Climb once the Konami code is entered; the menu still moves with the code.
     *
     * @return false, so the menu reads the actions too.
     */
    bool interceptInput(void) override;

    KonamiCode _konami;
};

} // namespace jtype

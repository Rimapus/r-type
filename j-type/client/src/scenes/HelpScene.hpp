/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/HelpScene
** File description:
** HelpScene header file
*/

#pragma once

#include <cstddef>

#include "scenes/Context.hpp"
#include "scenes/MenuScreen.hpp"

namespace jtype {

/**
 * @brief The help screen, from the main menu: the goal of the game and its controls, with the bindings of the player,
 *        see getHelpLines(). BACK, or back, goes back to the main menu.
 */
class HelpScene : public MenuScreen
{
public:
    /**
     * @brief Create the help screen, with the bindings of the settings of the context.
     *
     * @param context What the scenes share
     */
    explicit HelpScene(Context &context);

private:
    void confirm(std::size_t line) override;
    void back(void) override;
};

} // namespace jtype

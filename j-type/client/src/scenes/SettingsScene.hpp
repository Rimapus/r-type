/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/SettingsScene
** File description:
** SettingsScene header file
*/

#pragma once

#include <cstddef>

#include "scenes/Context.hpp"
#include "scenes/MenuScreen.hpp"

namespace jtype {

/**
 * @brief The settings screen: left and right change the volume of the music and of the sounds, heard at once, and
 *        switch the captions of the sounds, the auto fire, the high contrast and the reduced flashes on or off, and
 *        choose the color blindness the colors are corrected for.
 *        CONTROLS opens the bindings of the keys. BACK saves the settings and goes back to the main menu.
 */
class SettingsScene : public MenuScreen
{
public:
    /**
     * @brief Create the settings screen, with the settings of the context.
     *
     * @param context What the scenes share
     */
    explicit SettingsScene(Context &context);

private:
    void confirm(std::size_t line) override;
    void adjust(std::size_t line, int delta) override;
    void back(void) override;
};

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/MenuScreen
** File description:
** MenuScreen header file
*/

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "render/HudLayout.hpp"
#include "render/Overlay.hpp"
#include "render/Starfield.hpp"
#include "scenes/Context.hpp"
#include "scenes/Menu.hpp"
#include "younity/ecs/Scene.hpp"

namespace jtype {

/**
 * @brief A screen of the menus: lines to choose from over the scrolling starfield. Up and down select a line, the
 *        screen decides what confirm, left, right and back do.
 */
class MenuScreen : public younity::ecs::Scene
{
public:
    /**
     * @brief Read the actions of the menus, unless the screen intercepts them, then scroll the starfield.
     *
     * @param dt The duration of the step, in seconds
     */
    void update(float dt) override;

protected:
    /**
     * @brief Create the screen with its first line selected.
     *
     * @param context What the scenes share
     * @param lines The texts of the lines, from the top
     * @param top The top of the first line, in pixels of the game area
     */
    MenuScreen(Context &context, std::vector<std::string> lines, float top);

    /**
     * @brief Do what a line does, when it is confirmed.
     *
     * @param line The index of the selected line
     */
    virtual void confirm(std::size_t line) = 0;

    /**
     * @brief Change the value of a line, on left or right. Nothing by default.
     *
     * @param line The index of the selected line
     * @param delta -1 for left, 1 for right
     */
    virtual void adjust(std::size_t line, int delta);

    /**
     * @brief Leave the screen, on back. Nothing by default.
     */
    virtual void back(void);

    /**
     * @brief Read the input before the menu, e.g. to wait for a key. Nothing by default.
     *
     * @return true to keep the menu from reading the input at this step.
     */
    virtual bool interceptInput(void);

    /**
     * @brief Change the text of a line.
     *
     * @param line The index of the line, from the top
     * @param text Its new text
     */
    void setLine(std::size_t line, std::string text);

    /**
     * @brief Write a text that stays on the screen, like its title.
     *
     * @param text The text
     */
    void write(const HudText &text);

    /**
     * @brief Darken the starfield or not, as the high contrast setting asks.
     */
    void applyContrast(void);

    /**
     * @brief Get what the scenes share.
     *
     * @return The context given to the screen.
     */
    [[nodiscard]] Context &getContext(void);

private:
    /**
     * @brief Move through the lines on up and down, and give confirm, left, right and back to the screen.
     */
    void readMenu(void);

    /**
     * @brief Draw the lines again, after a change of the selection or of a text.
     */
    void drawLines(void);

    Context &_context;
    Starfield _starfield;
    Overlay _texts;
    Overlay _lines;
    Menu _menu;
};

} // namespace jtype

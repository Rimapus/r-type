/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Menu
** File description:
** Menu header file
*/

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "render/BitmapFont.hpp"
#include "render/HudLayout.hpp"

namespace jtype {

/**
 * @brief Lines to choose from, one under the other in the middle of the screen, one of them selected.
 */
class Menu
{
public:
    /**
     * @brief Create a menu with its first line selected.
     *
     * @param lines The texts of the lines, from the top
     * @param top The top of the first line, in pixels of the game area
     *
     * @throw std::invalid_argument if there is no line.
     */
    Menu(std::vector<std::string> lines, float top);

    /**
     * @brief Select the line above or below, going around at the ends.
     *
     * @param delta -1 for the line above, 1 for the line below
     */
    void move(int delta);

    /**
     * @brief Change the text of a line, e.g. when its value changes.
     *
     * @param line The index of the line, from the top
     * @param text Its new text
     */
    void setLine(std::size_t line, std::string text);

    /**
     * @brief Get the selected line.
     *
     * @return Its index, from the top.
     */
    [[nodiscard]] std::size_t getSelected(void) const;

    /**
     * @brief Lay out the lines, each one centered: the selected one amber and between dashes, the others white.
     *
     * @return The texts to draw.
     */
    [[nodiscard]] std::vector<HudText> layout(void) const;

private:
    std::vector<std::string> _lines;
    float _top;
    std::size_t _selected = 0;
};

/**
 * @brief Place a text in the middle of the width of the game area.
 *
 * @param string The text
 * @param top The top of the text, in pixels of the game area
 * @param color Its color
 * @param size The times the font is drawn bigger than the game area
 *
 * @return The text placed.
 */
[[nodiscard]] HudText centerText(std::string string, float top, FontColor color, float size);

} // namespace jtype

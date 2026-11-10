/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Menu
** File description:
** Menu source file
*/

#include "scenes/Menu.hpp"

#include <stdexcept>
#include <utility>

#include "jtype/common/Constants.hpp"
#include "younity/core/Vector2.hpp"

namespace jtype {

namespace {

// The times the font of the lines is drawn bigger than the game area, twice the size of the HUD
constexpr float LINE_SIZE = 2.0F;

// From the top of a line to the top of the next one, in pixels of the game area
constexpr float LINE_SPACING = 24.0F;

} // namespace

Menu::Menu(std::vector<std::string> lines, float top)
    : _lines(std::move(lines)),
      _top(top)
{
    if (this->_lines.empty()) {
        throw std::invalid_argument("a menu needs a line");
    }
}

void Menu::move(int delta)
{
    const auto count = static_cast<int>(this->_lines.size());
    const int selected = (static_cast<int>(this->_selected) + delta % count + count) % count;

    this->_selected = static_cast<std::size_t>(selected);
}

void Menu::setLine(std::size_t line, std::string text)
{
    this->_lines.at(line) = std::move(text);
}

std::size_t Menu::getSelected(void) const
{
    return this->_selected;
}

std::vector<HudText> Menu::layout(void) const
{
    std::vector<HudText> texts;

    for (std::size_t line = 0; line < this->_lines.size(); ++line) {
        const float top = this->_top + LINE_SPACING * static_cast<float>(line);

        // The dashes tell the selected line apart without its color
        if (line == this->_selected) {
            texts.push_back(centerText("- " + this->_lines[line] + " -", top, FontColor::Amber, LINE_SIZE));
        } else {
            texts.push_back(centerText(this->_lines[line], top, FontColor::White, LINE_SIZE));
        }
    }
    return texts;
}

HudText centerText(std::string string, float top, FontColor color, float size)
{
    const float width = static_cast<float>(string.size()) * GLYPH_ADVANCE * size;
    const younity::Vector2f topLeft{(WORLD_WIDTH - width) / 2.0F, top};

    return HudText{std::move(string), topLeft, color, size};
}

} // namespace jtype

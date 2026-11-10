/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/BitmapFont
** File description:
** BitmapFont header file
*/

#pragma once

#include <string_view>
#include <vector>

#include "younity/core/Rect.hpp"

namespace jtype {

/**
 * @brief A color of the 8x8 font, one image each.
 */
enum class FontColor {
    White,
    Amber
};

// The size of a cell of the font, in pixels of the game area
inline constexpr int GLYPH_SIZE = 8;

// The distance from a character to the next one: the cell without its shadow column
inline constexpr float GLYPH_ADVANCE = 7.0F;

/**
 * @brief A character of a text, placed in the font.
 */
struct Glyph
{
    // The cell of the character in the image of the font
    younity::Recti frame;
    // From the left of the text, in pixels of the game area
    float x;
};

/**
 * @brief Find the image of the font of a color.
 *
 * @param color The color
 *
 * @return The file, relative to the assets folder.
 */
[[nodiscard]] const char *getFontPath(FontColor color);

/**
 * @brief Place the characters of a text one after the other.
 *        A lowercase letter is drawn uppercase. A space, or a character missing from the font, leaves a gap.
 *
 * @param text The text, in ASCII
 *
 * @return The characters to draw, from the left.
 */
[[nodiscard]] std::vector<Glyph> layoutText(std::string_view text);

} // namespace jtype

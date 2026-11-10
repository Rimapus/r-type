/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/BitmapFont
** File description:
** BitmapFont source file
*/

#include "render/BitmapFont.hpp"

#include <cctype>
#include <cstddef>

namespace jtype {

namespace {

// The characters of the font, in the order of its cells, the charset of its .json file without the blank cell
constexpr std::string_view CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.,!?:-+/%";

// The cells in a row of the image
constexpr std::size_t COLUMNS = 16;

} // namespace

const char *getFontPath(FontColor color)
{
    switch (color) {
    case FontColor::Amber:
        return "sprites/16bit/ui/font_8x8_amber.png";
    case FontColor::White:
        break;
    }
    return "sprites/16bit/ui/font_8x8_white.png";
}

std::vector<Glyph> layoutText(std::string_view text)
{
    std::vector<Glyph> glyphs;
    float x = 0.0F;

    for (const char character : text) {
        const std::size_t index = CHARSET.find(static_cast<char>(std::toupper(static_cast<unsigned char>(character))));

        if (index != std::string_view::npos) {
            const younity::Recti frame{static_cast<int>(index % COLUMNS) * GLYPH_SIZE, static_cast<int>(index / COLUMNS) * GLYPH_SIZE,
                                       GLYPH_SIZE, GLYPH_SIZE};

            glyphs.push_back(Glyph{frame, x});
        }
        x += GLYPH_ADVANCE;
    }
    return glyphs;
}

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/Text
** File description:
** Text unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <memory>

#include "../TestSupport.hpp"
#include "younity/render/Font.hpp"
#include "younity/render/components/Text.hpp"

using younity::Color;
using younity::Rectf;
using younity::render::Font;
using younity::render::Text;
using younity::test::skipWithoutDisplay;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "render" / "fixtures";

std::shared_ptr<const Font> loadTuffy(void)
{
    auto font = Font::loadFromFile(FIXTURES / "tuffy.ttf");

    cr_assert(font.has_value());
    return std::make_shared<const Font>(std::move(*font));
}

} // namespace

Test(render_text, new_text_has_default_values)
{
    const Text text;

    cr_assert_null(text.font);
    cr_assert(text.string.empty());
    cr_assert_eq(text.characterSize, 30U);
    cr_assert(text.color == (Color{255, 255, 255, 255}));
    cr_assert_eq(text.layer, 0);
    cr_assert(text.visible);
}

Test(render_text, color_is_opaque_by_default)
{
    const Color red{255, 0, 0};

    cr_assert_eq(red.a, 255);
}

Test(render_text, text_without_font_has_no_size)
{
    const Text text{.font = nullptr, .string = "no font"};

    cr_assert(younity::render::getTextBounds(text) == (Rectf{0.0F, 0.0F, 0.0F, 0.0F}));
}

Test(render_text, empty_text_has_no_width)
{
    skipWithoutDisplay();
    const Text text{.font = loadTuffy(), .string = ""};

    cr_assert_float_eq(younity::render::getTextBounds(text).width, 0.0F, 0.0001F);
}

Test(render_text, longer_text_is_wider)
{
    skipWithoutDisplay();
    const std::shared_ptr<const Font> font = loadTuffy();
    const Rectf short_ = younity::render::getTextBounds(Text{.font = font, .string = "AB"});
    const Rectf long_ = younity::render::getTextBounds(Text{.font = font, .string = "ABABAB"});

    cr_assert_gt(short_.width, 0.0F);
    cr_assert_gt(long_.width, short_.width);
}

Test(render_text, bigger_characters_are_taller)
{
    skipWithoutDisplay();
    const std::shared_ptr<const Font> font = loadTuffy();
    const Rectf small = younity::render::getTextBounds(Text{.font = font, .string = "Score", .characterSize = 10});
    const Rectf big = younity::render::getTextBounds(Text{.font = font, .string = "Score", .characterSize = 40});

    cr_assert_gt(big.height, small.height);
}

Test(render_text, accents_are_measured_as_utf8)
{
    skipWithoutDisplay();
    const std::shared_ptr<const Font> font = loadTuffy();
    // "é" is two bytes in UTF-8 but one character: it must not be two glyphs wide
    const Rectf accented = younity::render::getTextBounds(Text{.font = font, .string = "\xC3\xA9"});
    const Rectf plain = younity::render::getTextBounds(Text{.font = font, .string = "ee"});

    cr_assert_lt(accented.width, plain.width);
}

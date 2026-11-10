/*
** EPITECH PROJECT, 2026
** j-type/tests/client/BitmapFont
** File description:
** Unit tests of the bitmap font
*/

#include <criterion/criterion.h>

#include <vector>

#include "render/BitmapFont.hpp"

using jtype::Glyph;
using jtype::layoutText;
using younity::Recti;

Test(client_bitmap_font, places_each_character_after_the_previous_one)
{
    const std::vector<Glyph> glyphs = layoutText("AB");

    cr_assert_eq(glyphs.size(), 2U);
    cr_assert(glyphs[0].frame == (Recti{80, 0, 8, 8}));
    cr_assert(glyphs[1].frame == (Recti{88, 0, 8, 8}));
    cr_assert_float_eq(glyphs[0].x, 0.0F, 0.001F);
    cr_assert_float_eq(glyphs[1].x, jtype::GLYPH_ADVANCE, 0.001F);
}

Test(client_bitmap_font, finds_the_characters_of_every_row)
{
    cr_assert(layoutText("0")[0].frame == (Recti{0, 0, 8, 8}));
    cr_assert(layoutText("G")[0].frame == (Recti{0, 8, 8, 8}));
    cr_assert(layoutText("W")[0].frame == (Recti{0, 16, 8, 8}));
    cr_assert(layoutText("%")[0].frame == (Recti{96, 16, 8, 8}));
}

Test(client_bitmap_font, draws_lowercase_letters_uppercase)
{
    cr_assert(layoutText("out")[0].frame == layoutText("OUT")[0].frame);
}

Test(client_bitmap_font, leaves_a_gap_for_a_space_or_an_unknown_character)
{
    const std::vector<Glyph> spaced = layoutText("A B");
    const std::vector<Glyph> unknown = layoutText("A#B");

    cr_assert_eq(spaced.size(), 2U);
    cr_assert_float_eq(spaced[1].x, 2.0F * jtype::GLYPH_ADVANCE, 0.001F);
    cr_assert_eq(unknown.size(), 2U);
    cr_assert_float_eq(unknown[1].x, 2.0F * jtype::GLYPH_ADVANCE, 0.001F);
}

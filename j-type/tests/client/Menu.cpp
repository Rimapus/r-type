/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Menu
** File description:
** Unit tests of the lines of the menus
*/

#include <criterion/criterion.h>

#include <stdexcept>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "render/BitmapFont.hpp"
#include "scenes/Menu.hpp"

using jtype::FontColor;
using jtype::HudText;
using jtype::Menu;

Test(client_menu, selects_the_first_line)
{
    const Menu menu({"PLAY", "SETTINGS", "QUIT"}, 100.0F);

    cr_assert_eq(menu.getSelected(), 0U);
}

Test(client_menu, goes_around_at_the_ends)
{
    Menu menu({"PLAY", "SETTINGS", "QUIT"}, 100.0F);

    menu.move(-1);
    cr_assert_eq(menu.getSelected(), 2U);
    menu.move(1);
    cr_assert_eq(menu.getSelected(), 0U);
    menu.move(1);
    cr_assert_eq(menu.getSelected(), 1U);
}

Test(client_menu, marks_the_selected_line_without_only_its_color)
{
    Menu menu({"PLAY", "QUIT"}, 100.0F);

    menu.move(1);

    const std::vector<HudText> texts = menu.layout();

    cr_assert_eq(texts.size(), 2U);
    cr_assert_str_eq(texts[0].string.c_str(), "PLAY");
    cr_assert_eq(texts[0].color, FontColor::White);
    cr_assert_str_eq(texts[1].string.c_str(), "- QUIT -");
    cr_assert_eq(texts[1].color, FontColor::Amber);
}

Test(client_menu, puts_the_lines_one_under_the_other)
{
    const std::vector<HudText> texts = Menu({"PLAY", "SETTINGS", "QUIT"}, 100.0F).layout();

    cr_assert_float_eq(texts[0].position.y, 100.0F, 0.001F);
    cr_assert_gt(texts[1].position.y, texts[0].position.y);
    cr_assert_gt(texts[2].position.y, texts[1].position.y);
}

Test(client_menu, changes_the_text_of_a_line)
{
    Menu menu({"MUSIC 50%", "BACK"}, 100.0F);

    menu.setLine(0, "MUSIC 60%");

    const std::vector<HudText> texts = menu.layout();

    cr_assert_str_eq(texts[0].string.c_str(), "- MUSIC 60% -");
}

Test(client_menu, needs_a_line)
{
    cr_assert_throw(Menu({}, 100.0F), std::invalid_argument);
}

Test(client_center_text, puts_a_text_in_the_middle_of_the_width)
{
    const HudText text = jtype::centerText("J-TYPE", 40.0F, FontColor::Amber, 4.0F);
    const float width = 6.0F * jtype::GLYPH_ADVANCE * 4.0F;

    cr_assert_float_eq(text.position.x + width / 2.0F, jtype::WORLD_WIDTH / 2.0F, 0.001F);
    cr_assert_float_eq(text.position.y, 40.0F, 0.001F);
    cr_assert_float_eq(text.size, 4.0F, 0.001F);
}

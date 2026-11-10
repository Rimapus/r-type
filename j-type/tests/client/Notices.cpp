/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Notices
** File description:
** Notices unit tests
*/

#include <criterion/criterion.h>

#include <vector>

#include "render/HudLayout.hpp"
#include "scenes/Notices.hpp"
#include "younity/core/Color.hpp"

using jtype::HudText;
using jtype::Notices;
using younity::Color;

Test(client_notices, shows_a_notice_in_the_color_of_its_player)
{
    Notices notices;

    notices.announce("P2 DISCONNECTED", Color{96, 160, 255});
    cr_assert(notices.update(0.1F));

    const std::vector<HudText> texts = notices.layout();

    cr_assert_eq(texts.size(), 1U);
    cr_assert_eq(texts[0].string, "P2 DISCONNECTED");
    cr_assert(texts[0].tint == (Color{96, 160, 255}));
    cr_assert_not(notices.update(0.1F));
}

Test(client_notices, stacks_the_newest_below_and_keeps_three)
{
    Notices notices;

    for (const char *text : {"P1 DISCONNECTED", "P2 DISCONNECTED", "P3 DISCONNECTED", "P4 DISCONNECTED"}) {
        notices.announce(text, Color{});
    }

    const std::vector<HudText> texts = notices.layout();

    cr_assert_eq(texts.size(), jtype::MAX_NOTICES);
    cr_assert_eq(texts.front().string, "P2 DISCONNECTED");
    cr_assert_lt(texts.front().position.y, texts.back().position.y);
}

Test(client_notices, fades_out_then_goes)
{
    Notices notices;

    notices.announce("P1 DISCONNECTED", Color{});
    cr_assert(notices.update(jtype::NOTICE_DURATION - jtype::NOTICE_FADE / 2.0F));
    cr_assert_lt(notices.layout()[0].tint.a, 255);
    cr_assert(notices.update(jtype::NOTICE_FADE));
    cr_assert(notices.layout().empty());
}

/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Popups
** File description:
** Popups unit tests
*/

#include <criterion/criterion.h>

#include <vector>

#include "render/HudLayout.hpp"
#include "scenes/Popups.hpp"
#include "younity/core/Color.hpp"

using jtype::HudText;
using jtype::Popups;
using younity::Color;

Test(client_popups, shows_the_points_above_the_place_in_the_color_of_the_player)
{
    Popups popups;

    popups.add("+100", {200.0F, 100.0F}, Color{255, 96, 96});

    const std::vector<HudText> texts = popups.layout();

    cr_assert_eq(texts.size(), 1U);
    cr_assert_eq(texts[0].string, "+100");
    cr_assert_lt(texts[0].position.x, 200.0F);
    cr_assert_lt(texts[0].position.y, 100.0F);
    cr_assert(texts[0].tint == (Color{255, 96, 96}));
}

Test(client_popups, rises_and_fades_out_then_goes)
{
    Popups popups;

    popups.add("+100", {200.0F, 100.0F}, Color{});

    const HudText first = popups.layout()[0];

    cr_assert(popups.update(jtype::POPUP_DURATION / 2.0F));

    const HudText later = popups.layout()[0];

    cr_assert_lt(later.position.y, first.position.y);
    cr_assert_lt(later.tint.a, first.tint.a);
    cr_assert(popups.update(jtype::POPUP_DURATION));
    cr_assert(popups.layout().empty());
    cr_assert_not(popups.update(0.1F));
}

Test(client_popups, names_the_power_ups)
{
    cr_assert_eq(jtype::getPowerUpName(jtype::PowerUpType::ExtraLife), "+1 LIFE");
    cr_assert_eq(jtype::getPowerUpName(jtype::PowerUpType::Shield), "SHIELD");
    cr_assert_eq(jtype::getPowerUpName(jtype::PowerUpType::SpreadShot), "SPREAD");
}

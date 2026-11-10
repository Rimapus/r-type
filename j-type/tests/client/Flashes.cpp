/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Flashes
** File description:
** Flashes unit tests
*/

#include <criterion/criterion.h>

#include "jtype/common/EntityType.hpp"
#include "render/Flashes.hpp"
#include "younity/core/Color.hpp"

using jtype::EntityType;
using jtype::FlashLook;

Test(client_flashes, an_explosion_is_drawn_as_it_is_by_default)
{
    const FlashLook look = jtype::getFlashLook(EntityType::Explosion, false);

    cr_assert_float_eq(look.scale, 1.0F, 1e-6F);
    cr_assert(look.color == younity::Color{});
}

Test(client_flashes, an_explosion_is_smaller_and_dimmer_with_fewer_flashes)
{
    const FlashLook look = jtype::getFlashLook(EntityType::Explosion, true);

    cr_assert_lt(look.scale, 1.0F);
    cr_assert_lt(look.color.a, 255);
}

Test(client_flashes, the_ships_do_not_change_with_fewer_flashes)
{
    const FlashLook look = jtype::getFlashLook(EntityType::Player1, true);

    cr_assert_float_eq(look.scale, 1.0F, 1e-6F);
    cr_assert(look.color == younity::Color{});
}

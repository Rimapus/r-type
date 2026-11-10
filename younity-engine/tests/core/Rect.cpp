/*
** EPITECH PROJECT, 2026
** younity-engine/tests/core/Rect
** File description:
** Rect unit tests
*/

#include <criterion/criterion.h>

#include "younity/core/Rect.hpp"

using younity::Recti;
using younity::Vector2i;

Test(core_rect, contains_a_point_inside)
{
    const Recti rect{0, 0, 10, 10};

    cr_assert(rect.contains(Vector2i{5, 5}));
}

Test(core_rect, does_not_contain_a_point_outside)
{
    const Recti rect{0, 0, 10, 10};

    cr_assert_not(rect.contains(Vector2i{15, 5}));
    cr_assert_not(rect.contains(Vector2i{10, 10}));
}

Test(core_rect, overlapping_rects_intersect)
{
    const Recti a{0, 0, 10, 10};
    const Recti b{5, 5, 10, 10};

    cr_assert(a.intersects(b));
    cr_assert(b.intersects(a));
}

Test(core_rect, separated_rects_do_not_intersect)
{
    const Recti a{0, 0, 10, 10};
    const Recti b{20, 0, 10, 10};

    cr_assert_not(a.intersects(b));
}

Test(core_rect, rects_touching_by_an_edge_do_not_intersect)
{
    const Recti a{0, 0, 10, 10};
    const Recti b{10, 0, 10, 10};

    cr_assert_not(a.intersects(b));
}

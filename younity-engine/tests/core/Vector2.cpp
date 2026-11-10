/*
** EPITECH PROJECT, 2026
** younity-engine/tests/core/Vector2
** File description:
** Vector2 unit tests
*/

#include <criterion/criterion.h>

#include "younity/core/Vector2.hpp"

using younity::Vector2f;
using younity::Vector2i;

Test(core_vector2, default_is_zero)
{
    const Vector2i vector;

    cr_assert_eq(vector.x, 0);
    cr_assert_eq(vector.y, 0);
}

Test(core_vector2, add_and_subtract)
{
    const Vector2i a{1, 2};
    const Vector2i b{3, 5};

    cr_assert(a + b == (Vector2i{4, 7}));
    cr_assert(b - a == (Vector2i{2, 3}));
}

Test(core_vector2, multiply_and_divide)
{
    const Vector2i vector{2, 4};

    cr_assert(vector * 3 == (Vector2i{6, 12}));
    cr_assert(vector / 2 == (Vector2i{1, 2}));
}

Test(core_vector2, add_in_place)
{
    Vector2i position{1, 1};

    position += Vector2i{2, 3};
    cr_assert(position == (Vector2i{3, 4}));
}

Test(core_vector2, length)
{
    const Vector2f vector{3.0F, 4.0F};

    cr_assert_float_eq(vector.length(), 5.0F, 0.0001F);
}

Test(core_vector2, normalized_has_a_length_of_one)
{
    const Vector2f vector{3.0F, 4.0F};

    cr_assert_float_eq(vector.normalized().length(), 1.0F, 0.0001F);
}

Test(core_vector2, normalized_zero_stays_zero)
{
    const Vector2f zero;

    cr_assert(zero.normalized() == zero);
}

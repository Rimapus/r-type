/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/PixelPerfect
** File description:
** PixelPerfect unit tests
*/

#include <criterion/criterion.h>

#include <stdexcept>

#include "younity/render/PixelPerfect.hpp"

using younity::Rectf;
using younity::Vector2f;
using younity::render::getPixelPerfectArea;

Test(render_pixel_perfect, scales_by_the_largest_whole_factor)
{
    // 800 / 128 is 6.25 and 600 / 128 is 4.69: the factor is 4
    cr_assert(getPixelPerfectArea({128, 128}, {800, 600}) == (Rectf{144.0F, 44.0F, 512.0F, 512.0F}));
}

Test(render_pixel_perfect, fills_a_window_of_the_same_size)
{
    cr_assert(getPixelPerfectArea({320, 180}, {320, 180}) == (Rectf{0.0F, 0.0F, 320.0F, 180.0F}));
}

Test(render_pixel_perfect, fills_a_window_of_a_whole_multiple)
{
    cr_assert(getPixelPerfectArea({320, 180}, {1920, 1080}) == (Rectf{0.0F, 0.0F, 1920.0F, 1080.0F}));
}

Test(render_pixel_perfect, centers_on_a_whole_pixel)
{
    // 3 pixels are left on each axis: the game starts at 1, not 1.5
    cr_assert(getPixelPerfectArea({10, 10}, {13, 13}) == (Rectf{1.0F, 1.0F, 10.0F, 10.0F}));
}

Test(render_pixel_perfect, shrinks_in_a_smaller_window)
{
    cr_assert(getPixelPerfectArea({128, 128}, {64, 100}) == (Rectf{0.0F, 18.0F, 64.0F, 64.0F}));
}

Test(render_pixel_perfect, maps_a_pixel_of_the_window_to_the_logical_resolution)
{
    const Rectf area = getPixelPerfectArea({128, 128}, {800, 600});

    // The game starts at (144, 44), each logical pixel is 4 pixels of the window
    cr_assert(younity::render::mapPixelToLogical({144.0F, 44.0F}, area, {128, 128}) == (Vector2f{0.0F, 0.0F}));
    cr_assert(younity::render::mapPixelToLogical({152.0F, 64.0F}, area, {128, 128}) == (Vector2f{2.0F, 5.0F}));
}

Test(render_pixel_perfect, maps_a_logical_point_back_to_the_window)
{
    const Rectf area = getPixelPerfectArea({128, 128}, {800, 600});

    cr_assert(younity::render::mapLogicalToPixel({2.0F, 5.0F}, area, {128, 128}) == (Vector2f{152.0F, 64.0F}));
}

Test(render_pixel_perfect, refuses_an_empty_size)
{
    cr_assert_throw((void)getPixelPerfectArea({0, 128}, {800, 600}), std::invalid_argument);
    cr_assert_throw((void)getPixelPerfectArea({128, 128}, {800, 0}), std::invalid_argument);
}

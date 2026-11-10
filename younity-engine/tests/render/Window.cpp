/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/Window
** File description:
** Window unit tests
*/

#include <criterion/criterion.h>

#include <cmath>
#include <optional>

#include "../TestSupport.hpp"
#include "younity/render/Window.hpp"

using younity::Rectf;
using younity::Vector2f;
using younity::Vector2i;
using younity::render::ColorMatrix;
using younity::render::Window;
using younity::test::skipWithoutDisplay;

namespace {

// Check that two points are the same, but for the rounding of the view of the window
bool isNear(Vector2f point, Vector2f expected)
{
    return std::abs(point.x - expected.x) < 0.01F && std::abs(point.y - expected.y) < 0.01F;
}

} // namespace

Test(render_window, pixel_is_the_world_point_before_a_resize)
{
    skipWithoutDisplay();

    const Window window(320, 240, "Window");

    cr_assert(window.mapPixelToWorld(Vector2i{10, 20}) == (Vector2f{10.0F, 20.0F}));
}

Test(render_window, world_point_is_the_pixel_before_a_resize)
{
    skipWithoutDisplay();

    const Window window(320, 240, "Window");

    cr_assert(window.mapWorldToPixel(Vector2f{10.0F, 20.0F}) == (Vector2i{10, 20}));
}

Test(render_window, removing_the_color_matrix_always_works)
{
    skipWithoutDisplay();

    Window window(320, 240, "Window");

    cr_assert(window.setColorMatrix(std::nullopt));
}

Test(render_window, a_color_matrix_keeps_the_world_in_place)
{
    skipWithoutDisplay();

    Window window(320, 240, "Window");

    // Swaps the red and the blue of each pixel
    if (!window.setColorMatrix(ColorMatrix{0, 0, 1, 0, 1, 0, 1, 0, 0})) {
        cr_skip("the graphics card has no shaders");
    }
    window.clear();
    window.display();
    cr_assert(window.mapPixelToWorld(Vector2i{10, 20}) == (Vector2f{10.0F, 20.0F}));
    cr_assert(window.setColorMatrix(std::nullopt));
    window.clear();
    window.display();
}

Test(render_window, a_color_matrix_applies_to_the_logical_resolution)
{
    skipWithoutDisplay();

    Window window(320, 240, "Window");

    window.setLogicalResolution(160, 120);
    if (!window.setColorMatrix(ColorMatrix{0.5F, 0, 0, 0, 0.5F, 0, 0, 0, 0.5F})) {
        cr_skip("the graphics card has no shaders");
    }
    window.clear();
    window.display();
    cr_assert(window.mapPixelToWorld(Vector2i{160, 120}) == (Vector2f{80.0F, 60.0F}));
}

Test(render_window, a_game_area_keeps_the_proportions_of_the_world)
{
    skipWithoutDisplay();

    Window window(320, 240, "Window");

    // The world of 320x240 is shrunk to 160x120 to fit in the area, between bars of 60 pixels above and below
    window.setGameArea(Rectf{0.0F, 0.0F, 160.0F, 240.0F});
    cr_assert(isNear(window.mapPixelToWorld(Vector2i{0, 60}), Vector2f{0.0F, 0.0F}));
    cr_assert(isNear(window.mapPixelToWorld(Vector2i{80, 120}), Vector2f{160.0F, 120.0F}));
    cr_assert(window.mapWorldToPixel(Vector2f{160.0F, 120.0F}) == (Vector2i{80, 120}));
}

Test(render_window, a_game_area_holds_the_logical_resolution)
{
    skipWithoutDisplay();

    Window window(320, 240, "Window");

    window.setLogicalResolution(80, 60);
    // Twice the logical resolution fits in the area: each logical pixel is a square of 2 pixels, from (160, 0)
    window.setGameArea(Rectf{160.0F, 0.0F, 160.0F, 120.0F});
    cr_assert(window.mapPixelToWorld(Vector2i{160, 0}) == (Vector2f{0.0F, 0.0F}));
    cr_assert(window.mapPixelToWorld(Vector2i{240, 60}) == (Vector2f{40.0F, 30.0F}));
    cr_assert(window.mapWorldToPixel(Vector2f{40.0F, 30.0F}) == (Vector2i{240, 60}));
    window.clear();
    window.display();
}

Test(render_window, no_game_area_shows_the_game_in_the_whole_window)
{
    skipWithoutDisplay();

    Window window(320, 240, "Window");

    window.setGameArea(Rectf{0.0F, 0.0F, 160.0F, 120.0F});
    window.setGameArea(std::nullopt);
    window.clear();
    window.display();
    cr_assert(window.mapPixelToWorld(Vector2i{10, 20}) == (Vector2f{10.0F, 20.0F}));
}

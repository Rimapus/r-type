/*
** EPITECH PROJECT, 2026
** younity-engine/tests/editor/Layout
** File description:
** Layout unit tests
*/

#include <criterion/criterion.h>

#include <array>
#include <cstddef>

#include "younity/editor/Layout.hpp"

using younity::Rectf;
using younity::Vector2f;
using younity::editor::DebugAreas;
using younity::editor::EditorAreas;
using younity::editor::getDebugAreas;
using younity::editor::getEditorAreas;

namespace {

const Vector2f WINDOW{1280.0F, 720.0F};

// Check that a rectangle is inside the window
bool isInWindow(const Rectf &area)
{
    return area.left >= 0.0F && area.top >= 0.0F && area.left + area.width <= WINDOW.x &&
           area.top + area.height <= WINDOW.y && area.width > 0.0F && area.height > 0.0F;
}

// Check that no two rectangles overlap, and that each one is inside the window
template <std::size_t Count>
bool areSeparate(const std::array<Rectf, Count> &areas)
{
    for (std::size_t first = 0; first < Count; ++first) {
        if (!isInWindow(areas[first])) {
            return false;
        }
        for (std::size_t second = first + 1; second < Count; ++second) {
            if (areas[first].intersects(areas[second])) {
                return false;
            }
        }
    }
    return true;
}

} // namespace

Test(editor_layout, debug_windows_stay_on_the_edges)
{
    const DebugAreas areas = getDebugAreas(WINDOW);

    cr_assert(areSeparate(std::array{areas.younity, areas.metrics, areas.console}));
    // The middle of the game stays visible
    cr_assert_not(areas.younity.contains(WINDOW / 2.0F));
    cr_assert_not(areas.metrics.contains(WINDOW / 2.0F));
    cr_assert_not(areas.console.contains(WINDOW / 2.0F));
}

Test(editor_layout, editor_windows_cover_the_window_around_the_preview)
{
    const EditorAreas areas = getEditorAreas(WINDOW);
    const std::array all{areas.preview, areas.entities, areas.scene, areas.inspector, areas.console, areas.assets, areas.metrics};
    float covered = 0.0F;

    for (const Rectf &area : all) {
        covered += area.width * area.height;
    }
    cr_assert(areSeparate(all));
    cr_assert_float_eq(covered, WINDOW.x * WINDOW.y, 1.0F);
}

Test(editor_layout, editor_preview_is_at_the_top_center)
{
    const EditorAreas areas = getEditorAreas(WINDOW);

    cr_assert_float_eq(areas.preview.top, 0.0F, 0.0001F);
    cr_assert_float_eq(areas.preview.left + (areas.preview.width / 2.0F), WINDOW.x / 2.0F, 0.0001F);
    cr_assert_lt(areas.preview.width, WINDOW.x);
    cr_assert_lt(areas.preview.height, WINDOW.y);
}

Test(editor_layout, follows_the_size_of_the_window)
{
    const EditorAreas small = getEditorAreas(WINDOW / 2.0F);
    const EditorAreas large = getEditorAreas(WINDOW);

    cr_assert_float_eq(small.preview.width * 2.0F, large.preview.width, 0.0001F);
    cr_assert_float_eq(small.console.top * 2.0F, large.console.top, 0.0001F);
}

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/DebugWindow
** File description:
** DebugWindow unit tests
*/

#include <criterion/criterion.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <optional>

#include "../TestSupport.hpp"
#include "younity/render/DebugWindow.hpp"
#include "younity/render/Window.hpp"

using younity::Rectf;
using younity::render::beginDebugWindow;
using younity::render::DEBUG_WINDOW_TITLE;
using younity::render::drawDebugWindow;
using younity::render::Window;
using younity::test::createContext;
using younity::test::destroyContext;
using younity::test::isFixedAt;
using younity::test::skipWithoutDisplay;

Test(render_debug_window, adds_the_debug_window, .init = createContext, .fini = destroyContext)
{
    ImGui::NewFrame();
    drawDebugWindow();
    ImGui::Render();

    const ImGuiWindow *window = ImGui::FindWindowByName(DEBUG_WINDOW_TITLE);

    cr_assert_not_null(window);
    cr_assert(window->Active);
}

Test(render_debug_window, is_absent_from_a_frame_that_skips_it, .init = createContext, .fini = destroyContext)
{
    ImGui::NewFrame();
    drawDebugWindow();
    ImGui::Render();
    ImGui::NewFrame();
    ImGui::Render();

    const ImGuiWindow *window = ImGui::FindWindowByName(DEBUG_WINDOW_TITLE);

    cr_assert_not_null(window);
    cr_assert_not(window->Active);
}

Test(render_debug_window, is_fixed_in_its_area, .init = createContext, .fini = destroyContext)
{
    ImGui::NewFrame();
    drawDebugWindow(Rectf{10.0F, 20.0F, 200.0F, 100.0F});
    ImGui::Render();

    cr_assert(isFixedAt(DEBUG_WINDOW_TITLE, Rectf{10.0F, 20.0F, 200.0F, 100.0F}));
}

Test(render_debug_window, without_an_area_is_moved_by_the_mouse, .init = createContext, .fini = destroyContext)
{
    ImGui::NewFrame();
    if (beginDebugWindow("Free", std::nullopt)) {
        ImGui::TextUnformatted("Moved by the mouse");
    }
    ImGui::End();
    ImGui::Render();

    const ImGuiWindow *window = ImGui::FindWindowByName("Free");

    cr_assert_not_null(window);
    cr_assert_eq(window->Flags & (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize), 0);
}

Test(render_debug_window, a_folded_window_unfolds_in_its_area, .init = createContext, .fini = destroyContext)
{
    ImGui::NewFrame();
    ImGui::SetNextWindowCollapsed(true);
    drawDebugWindow();
    ImGui::Render();
    ImGui::NewFrame();
    drawDebugWindow(Rectf{0.0F, 0.0F, 200.0F, 100.0F});
    ImGui::Render();
    cr_assert_not(ImGui::FindWindowByName(DEBUG_WINDOW_TITLE)->Collapsed);
}

Test(render_debug_window, draws_in_a_window_frame)
{
    skipWithoutDisplay();

    Window window(320, 240, "Debug window");

    window.pollEvents();
    window.clear();
    drawDebugWindow();
    window.display();
    cr_assert(window.isOpen());
}

Test(render_debug_window, opens_one_frame_per_display)
{
    skipWithoutDisplay();

    Window window(320, 240, "Debug window");

    // A second clear() keeps the frame already open, and a display() without clear() draws the game alone
    window.clear();
    window.clear();
    drawDebugWindow();
    window.display();
    window.display();
    cr_assert(window.isOpen());
}

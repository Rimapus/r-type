/*
** EPITECH PROJECT, 2026
** younity-engine/tests/TestSupport
** File description:
** Helpers shared by the tests
*/

#pragma once

#include <criterion/criterion.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cmath>
#include <cstdlib>

#include "younity/core/Rect.hpp"

namespace younity::test {

/**
 * @brief Create a debug interface without a window, its frames built in memory.
 */
inline void createContext(void)
{
    unsigned char *pixels = nullptr;
    int width = 0;
    int height = 0;

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(800.0F, 600.0F);
    io.DeltaTime = 1.0F / 60.0F;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}

/**
 * @brief Destroy the debug interface created by createContext().
 */
inline void destroyContext(void)
{
    ImGui::DestroyContext();
}

/**
 * @brief Draw one frame of a window exposing draw().
 *
 * @param window The window to draw
 */
template <typename Window>
void drawFrame(Window &window)
{
    ImGui::NewFrame();
    window.draw();
    ImGui::Render();
}

/**
 * @brief Check that a window of the debug interface was drawn in the last frame.
 *
 * @param title The title of the window
 *
 * @return true if the window exists and is active.
 */
inline bool isDrawn(const char *title)
{
    const ImGuiWindow *window = ImGui::FindWindowByName(title);

    return window != nullptr && window->Active;
}

/**
 * @brief Check that a window of the debug interface is fixed in an area: placed and sized by it, and not moved,
 *        resized or folded by the mouse.
 *
 * @param title The title of the window
 * @param area The area it should fill
 *
 * @return true if the window exists, fills the area and is fixed.
 */
inline bool isFixedAt(const char *title, const Rectf &area)
{
    const ImGuiWindow *window = ImGui::FindWindowByName(title);
    const ImGuiWindowFlags fixed = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    return window != nullptr && std::abs(window->Pos.x - area.left) < 0.5F && std::abs(window->Pos.y - area.top) < 0.5F &&
           std::abs(window->Size.x - area.width) < 0.5F && std::abs(window->Size.y - area.height) < 0.5F &&
           (window->Flags & fixed) == fixed;
}

/**
 * @brief Skip a test that needs a window when no display is available.
 */
inline void skipWithoutDisplay(void)
{
#if defined(__linux__)
    if (std::getenv("DISPLAY") == nullptr && std::getenv("WAYLAND_DISPLAY") == nullptr) {
        cr_skip_test("no display to open a window");
    }
#endif
}

} // namespace younity::test

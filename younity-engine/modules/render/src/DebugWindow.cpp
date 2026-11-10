/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/DebugWindow
** File description:
** DebugWindow source file
*/

#include "younity/render/DebugWindow.hpp"
#include "younity/core/Version.hpp"
#include <imgui.h>
#include <optional>

namespace younity::render {

bool beginDebugWindow(const char *title, const std::optional<Rectf> &area)
{
    if (!area) {
        return ImGui::Begin(title);
    }
    ImGui::SetNextWindowPos(ImVec2(area->left, area->top), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(area->width, area->height), ImGuiCond_Always);
    // A window folded before it was fixed could not be unfolded anymore
    ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
    return ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
}

void drawDebugWindow(const std::optional<Rectf> &area)
{
    // Average over the last 60 frames of the debug interface: steadier than the time of the last frame
    const float framerate = ImGui::GetIO().Framerate;

    if (beginDebugWindow(DEBUG_WINDOW_TITLE, area)) {
        ImGui::Text("Version %.*s", static_cast<int>(version().size()), version().data());
        ImGui::Text("%.0f FPS", framerate);
        ImGui::Text("%.2f ms per frame", framerate > 0.0F ? 1000.0F / framerate : 0.0F);
    }
    ImGui::End();
}

} // namespace younity::render

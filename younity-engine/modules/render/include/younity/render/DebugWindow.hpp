/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/DebugWindow
** File description:
** DebugWindow header file
*/

#pragma once

#include <optional>

#include "younity/core/Rect.hpp"

namespace younity::render {

/**
 * @brief Title of the debug window, also its id in the debug interface.
 */
inline constexpr const char *DEBUG_WINDOW_TITLE = "Younity";

/**
 * @brief Begin a window of the debug interface, like ImGui::Begin(), fixed in an area of the window when one is given:
 *        the mouse can then not move, resize or fold it. End it with ImGui::End(), whatever this returns.
 *
 * @param title The title of the window, also its id
 * @param area Where the window is fixed, in pixels from the top left corner of the window, or nothing to let the mouse
 *        move and resize it
 *
 * @return false if the window is folded or hidden: its content can be skipped.
 */
[[nodiscard]] bool beginDebugWindow(const char *title, const std::optional<Rectf> &area);

/**
 * @brief Draw the debug window of the engine over the game: the version of the engine, the frame rate
 *        and the frame time. Called once per frame between window.clear() and window.display().
 *
 * @param area Where the window is fixed, in pixels from the top left corner of the window, or nothing to let the mouse
 *        move and resize it
 */
void drawDebugWindow(const std::optional<Rectf> &area = std::nullopt);

} // namespace younity::render

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/include/younity/editor/Layout
** File description:
** Layout header file
*/

#pragma once

#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"

namespace younity::editor {

/**
 * @brief What the window shows with the game, see Workspace.
 */
enum class Layout {
    /** @brief The game in the whole window, without the debug interface. */
    Game,
    /** @brief The game in the whole window, with the Younity, Metrics and Console windows fixed on its edges. */
    Debug,
    /** @brief The editor: the preview of the game at the top center, the windows of the editor fixed around it. */
    Editor,
};

/**
 * @brief Where the windows of the debug layout are fixed, in pixels from the top left corner of the window.
 */
struct DebugAreas
{
    Rectf younity;
    Rectf metrics;
    Rectf console;
};

/**
 * @brief Where the preview of the game and the windows of the editor layout are, in pixels from the top left corner of
 *        the window. Together they cover the window, without overlapping.
 */
struct EditorAreas
{
    Rectf preview;
    Rectf entities;
    Rectf scene;
    Rectf inspector;
    Rectf console;
    Rectf assets;
    Rectf metrics;
};

/**
 * @brief Place the windows of the debug layout on the edges of the game: Younity at the top left, Metrics at the top
 *        right and Console along the bottom.
 *
 * @param windowSize The size of the window, in pixels
 *
 * @return Where each window is fixed.
 */
[[nodiscard]] DebugAreas getDebugAreas(Vector2f windowSize) noexcept;

/**
 * @brief Place the editor layout: the preview of the game at the top center, Entities over Scene on its left, Inspector
 *        on its right, and Console, Assets and Metrics along the bottom.
 *
 * @param windowSize The size of the window, in pixels
 *
 * @return Where the preview and each window are.
 */
[[nodiscard]] EditorAreas getEditorAreas(Vector2f windowSize) noexcept;

} // namespace younity::editor

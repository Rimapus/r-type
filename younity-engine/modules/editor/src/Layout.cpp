/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/src/Layout
** File description:
** Layout source file
*/

#include "younity/editor/Layout.hpp"

namespace younity::editor {

namespace {

// The part of the window each area takes, from 0 to 1

// Debug layout: the game stays visible in the middle
constexpr float DEBUG_YOUNITY_WIDTH = 0.22F;
constexpr float DEBUG_YOUNITY_HEIGHT = 0.2F;
constexpr float DEBUG_METRICS_WIDTH = 0.4F;
constexpr float DEBUG_METRICS_HEIGHT = 0.35F;
constexpr float DEBUG_CONSOLE_HEIGHT = 0.25F;

// Editor layout: a column on each side of the preview, and a row under them
constexpr float EDITOR_COLUMN_WIDTH = 0.22F;
constexpr float EDITOR_TOP_HEIGHT = 0.7F;
constexpr float EDITOR_ENTITIES_HEIGHT = 0.5F;
constexpr float EDITOR_CONSOLE_WIDTH = 0.4F;
constexpr float EDITOR_ASSETS_WIDTH = 0.2F;

} // namespace

DebugAreas getDebugAreas(Vector2f windowSize) noexcept
{
    const float width = windowSize.x;
    const float height = windowSize.y;
    const float consoleHeight = height * DEBUG_CONSOLE_HEIGHT;
    DebugAreas areas;

    areas.younity = Rectf{0.0F, 0.0F, width * DEBUG_YOUNITY_WIDTH, height * DEBUG_YOUNITY_HEIGHT};
    areas.metrics = Rectf{width * (1.0F - DEBUG_METRICS_WIDTH), 0.0F, width * DEBUG_METRICS_WIDTH, height * DEBUG_METRICS_HEIGHT};
    areas.console = Rectf{0.0F, height - consoleHeight, width, consoleHeight};
    return areas;
}

EditorAreas getEditorAreas(Vector2f windowSize) noexcept
{
    const float width = windowSize.x;
    const float height = windowSize.y;
    const float column = width * EDITOR_COLUMN_WIDTH;
    const float top = height * EDITOR_TOP_HEIGHT;
    const float entities = height * EDITOR_ENTITIES_HEIGHT;
    const float console = width * EDITOR_CONSOLE_WIDTH;
    const float assets = width * EDITOR_ASSETS_WIDTH;
    EditorAreas areas;

    areas.preview = Rectf{column, 0.0F, width - (2.0F * column), top};
    areas.entities = Rectf{0.0F, 0.0F, column, entities};
    areas.scene = Rectf{0.0F, entities, column, top - entities};
    areas.inspector = Rectf{width - column, 0.0F, column, top};
    areas.console = Rectf{0.0F, top, console, height - top};
    areas.assets = Rectf{console, top, assets, height - top};
    areas.metrics = Rectf{console + assets, top, width - console - assets, height - top};
    return areas;
}

} // namespace younity::editor

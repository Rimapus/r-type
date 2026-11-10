/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/include/younity/editor/Workspace
** File description:
** Workspace header file
*/

#pragma once

#include <optional>

#include "younity/ecs/Registry.hpp"
#include "younity/editor/AssetBrowser.hpp"
#include "younity/editor/ConsoleWindow.hpp"
#include "younity/editor/Editor.hpp"
#include "younity/editor/Layout.hpp"
#include "younity/editor/MetricsWindow.hpp"
#include "younity/input/Key.hpp"
#include "younity/render/Window.hpp"

namespace younity::editor {

/**
 * @brief The layouts of the debug interface, switched by keys the game chooses: the game alone, the game with the debug
 *        windows fixed on its edges, or the editor with the preview of the game at the top center. The windows of a
 *        layout are fixed where getDebugAreas() and getEditorAreas() place them, again at each frame, so a resized
 *        window keeps them in place. The debug and the editor layouts are never shown together.
 */
class Workspace
{
public:
    /**
     * @brief Lay out the windows of a game, starting with the game layout. The windows must outlive the workspace.
     *
     * @param editor The entity list, the inspector and the scene window
     * @param console The console window
     * @param metrics The metrics window
     * @param assets The asset browser
     */
    Workspace(Editor &editor, ConsoleWindow &console, MetricsWindow &metrics, AssetBrowser &assets) noexcept;

    /**
     * @brief Choose the key that shows a layout: pressed in another layout it shows this one, pressed again it goes back
     *        to the game layout. The key replaces the one chosen before for this layout.
     *
     * @param layout Layout::Debug or Layout::Editor
     * @param key The key, e.g. F1
     *
     * @throw std::invalid_argument for Layout::Game: the key of the layout shown goes back to it.
     */
    void bindKey(Layout layout, input::Key key);

    /**
     * @brief Show a layout, or the game layout if it is already shown, as its key does.
     *
     * @param layout The layout to show
     */
    void toggle(Layout layout) noexcept;

    /**
     * @brief Get the layout shown.
     *
     * @return The layout, Layout::Game at the creation.
     */
    [[nodiscard]] Layout getLayout(void) const noexcept;

    /**
     * @brief Tell if the game reads the keyboard and the gamepad, e.g. if (workspace.hasGameInput()) { move the player }.
     *        The game keeps them in the game layout. In the debug layout it loses them while a field of the debug
     *        interface is typed in, like the command line of the console. In the editor layout it gets them once the
     *        preview is clicked, until a window of the editor is clicked.
     *
     * @return true if the game reads its inputs.
     */
    [[nodiscard]] bool hasGameInput(void) const;

    /**
     * @brief Switch the layout when one of its keys was pressed, show the game in its area, the whole window or the
     *        preview of the editor layout, and draw the windows of the layout at their places. Called once per frame
     *        between window.clear() and window.display(), after the game is drawn.
     *
     * @param registry The registry the editor shows, e.g. the one of the active scene
     * @param window The window the game is drawn in
     */
    void draw(ecs::Registry &registry, render::Window &window);

private:
    /**
     * @brief Give the inputs to the game when the preview is clicked, and back to the editor when one of its windows is.
     *
     * @param preview Where the preview is in the window
     */
    void focusPreview(const Rectf &preview);

    Editor &_editor;
    ConsoleWindow &_console;
    MetricsWindow &_metrics;
    AssetBrowser &_assets;
    Layout _layout = Layout::Game;
    std::optional<input::Key> _debugKey;
    std::optional<input::Key> _editorKey;
    // The preview of the editor layout has the inputs of the game, from a click in it to a click on a window
    bool _isPreviewFocused = false;
};

} // namespace younity::editor

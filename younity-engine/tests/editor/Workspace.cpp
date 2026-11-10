/*
** EPITECH PROJECT, 2026
** younity-engine/tests/editor/Workspace
** File description:
** Workspace unit tests
*/

#include <criterion/criterion.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <filesystem>
#include <stdexcept>

#include "../TestSupport.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/editor/AssetBrowser.hpp"
#include "younity/editor/ConsoleWindow.hpp"
#include "younity/editor/Editor.hpp"
#include "younity/editor/Layout.hpp"
#include "younity/editor/MetricsWindow.hpp"
#include "younity/editor/Workspace.hpp"
#include "younity/input/Key.hpp"
#include "younity/logger/LoggerRegistry.hpp"
#include "younity/render/DebugWindow.hpp"
#include "younity/render/Window.hpp"
#include "younity/tools/Console.hpp"
#include "younity/tools/MetricsStore.hpp"

using younity::Vector2f;
using younity::assets::ResourceManager;
using younity::ecs::ComponentRegistry;
using younity::ecs::Registry;
using younity::editor::ASSET_BROWSER_TITLE;
using younity::editor::AssetBrowser;
using younity::editor::CONSOLE_TITLE;
using younity::editor::ConsoleWindow;
using younity::editor::DebugAreas;
using younity::editor::Editor;
using younity::editor::EditorAreas;
using younity::editor::ENTITY_LIST_TITLE;
using younity::editor::getDebugAreas;
using younity::editor::getEditorAreas;
using younity::editor::INSPECTOR_TITLE;
using younity::editor::Layout;
using younity::editor::METRICS_TITLE;
using younity::editor::MetricsWindow;
using younity::editor::SCENE_TITLE;
using younity::editor::Workspace;
using younity::input::Key;
using younity::logger::LoggerRegistry;
using younity::render::DEBUG_WINDOW_TITLE;
using younity::render::Window;
using younity::test::isDrawn;
using younity::test::isFixedAt;
using younity::test::skipWithoutDisplay;
using younity::tools::Console;
using younity::tools::MetricsStore;

namespace {

const Vector2f SIZE{800.0F, 600.0F};

/**
 * @brief The windows of a game, laid out by a workspace with F1 for the debug layout and F2 for the editor layout.
 */
struct Game
{
    Window window{800, 600, "Workspace"};
    Registry registry;
    ComponentRegistry components;
    Editor editor{components};
    Console console;
    LoggerRegistry logs;
    ConsoleWindow consoleWindow{console, logs};
    MetricsStore store;
    MetricsWindow metrics{store};
    ResourceManager resources{std::filesystem::path(YOUNITY_TESTS_DIR) / "render" / "fixtures"};
    AssetBrowser assets{resources};
    Workspace workspace{editor, consoleWindow, metrics, assets};

    Game(void)
    {
        ImGuiIO &io = ImGui::GetIO();

        // A mouse move and a click in the same frame are both seen in this frame
        io.ConfigInputTrickleEventQueue = false;
        io.DisplaySize = ImVec2(SIZE.x, SIZE.y);
        this->workspace.bindKey(Layout::Debug, Key::F1);
        this->workspace.bindKey(Layout::Editor, Key::F2);
    }

    // Draw one frame of the workspace. window.clear() is skipped: it would read the real mouse over the one of the test
    void drawFrame(void)
    {
        ImGui::NewFrame();
        this->workspace.draw(this->registry, this->window);
        ImGui::Render();
    }

    // Press a key and release it, in two frames
    void press(ImGuiKey key)
    {
        ImGui::GetIO().AddKeyEvent(key, true);
        this->drawFrame();
        ImGui::GetIO().AddKeyEvent(key, false);
        this->drawFrame();
    }

    // Click at a point of the window, in two frames
    void click(float x, float y)
    {
        ImGuiIO &io = ImGui::GetIO();

        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        this->drawFrame();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        this->drawFrame();
    }
};

} // namespace

Test(editor_workspace, starts_with_the_game_alone)
{
    skipWithoutDisplay();

    Game game;

    game.drawFrame();
    cr_assert(game.workspace.getLayout() == Layout::Game);
    cr_assert_not(isDrawn(DEBUG_WINDOW_TITLE));
    cr_assert_not(isDrawn(ENTITY_LIST_TITLE));
    cr_assert(game.workspace.hasGameInput());
}

Test(editor_workspace, debug_key_fixes_the_debug_windows_on_the_edges)
{
    skipWithoutDisplay();

    Game game;
    const DebugAreas areas = getDebugAreas(SIZE);

    game.press(ImGuiKey_F1);
    cr_assert(game.workspace.getLayout() == Layout::Debug);
    cr_assert(isFixedAt(DEBUG_WINDOW_TITLE, areas.younity));
    cr_assert(isFixedAt(METRICS_TITLE, areas.metrics));
    cr_assert(isFixedAt(CONSOLE_TITLE, areas.console));
    cr_assert_not(isDrawn(ENTITY_LIST_TITLE));
    cr_assert(game.workspace.hasGameInput());
}

Test(editor_workspace, editor_key_fixes_the_editor_windows_around_the_preview)
{
    skipWithoutDisplay();

    Game game;
    const EditorAreas areas = getEditorAreas(SIZE);

    game.press(ImGuiKey_F2);
    cr_assert(game.workspace.getLayout() == Layout::Editor);
    cr_assert(isFixedAt(ENTITY_LIST_TITLE, areas.entities));
    cr_assert(isFixedAt(SCENE_TITLE, areas.scene));
    cr_assert(isFixedAt(INSPECTOR_TITLE, areas.inspector));
    cr_assert(isFixedAt(CONSOLE_TITLE, areas.console));
    cr_assert(isFixedAt(ASSET_BROWSER_TITLE, areas.assets));
    cr_assert(isFixedAt(METRICS_TITLE, areas.metrics));
    cr_assert_not(isDrawn(DEBUG_WINDOW_TITLE));
}

Test(editor_workspace, editor_shows_the_game_in_the_preview)
{
    skipWithoutDisplay();

    Game game;
    const EditorAreas areas = getEditorAreas(SIZE);

    game.press(ImGuiKey_F2);
    // The world of 800x600 keeps its proportions in the preview: its top left corner is on the left edge of the preview
    cr_assert_float_eq(game.window.mapPixelToWorld({static_cast<int>(areas.preview.left), 0}).x, 0.0F, 2.0F);
    game.press(ImGuiKey_F2);
    cr_assert(game.window.mapPixelToWorld({10, 20}) == (Vector2f{10.0F, 20.0F}));
}

Test(editor_workspace, key_pressed_again_goes_back_to_the_game)
{
    skipWithoutDisplay();

    Game game;

    game.press(ImGuiKey_F1);
    game.press(ImGuiKey_F1);
    cr_assert(game.workspace.getLayout() == Layout::Game);
    cr_assert_not(isDrawn(DEBUG_WINDOW_TITLE));
}

Test(editor_workspace, other_key_switches_straight_to_its_layout)
{
    skipWithoutDisplay();

    Game game;

    game.press(ImGuiKey_F1);
    game.press(ImGuiKey_F2);
    cr_assert(game.workspace.getLayout() == Layout::Editor);
    cr_assert_not(isDrawn(DEBUG_WINDOW_TITLE));
    game.press(ImGuiKey_F1);
    cr_assert(game.workspace.getLayout() == Layout::Debug);
    cr_assert_not(isDrawn(ENTITY_LIST_TITLE));
}

Test(editor_workspace, toggle_shows_a_layout_and_hides_it)
{
    skipWithoutDisplay();

    Game game;

    game.workspace.toggle(Layout::Editor);
    cr_assert(game.workspace.getLayout() == Layout::Editor);
    game.workspace.toggle(Layout::Editor);
    cr_assert(game.workspace.getLayout() == Layout::Game);
}

Test(editor_workspace, game_layout_has_no_key)
{
    skipWithoutDisplay();

    Game game;

    cr_assert_throw(game.workspace.bindKey(Layout::Game, Key::F3), std::invalid_argument);
}

Test(editor_workspace, editor_gives_the_inputs_to_the_preview_once_clicked)
{
    skipWithoutDisplay();

    Game game;
    const EditorAreas areas = getEditorAreas(SIZE);

    game.press(ImGuiKey_F2);
    cr_assert_not(game.workspace.hasGameInput());
    game.click(areas.preview.left + (areas.preview.width / 2.0F), areas.preview.top + (areas.preview.height / 2.0F));
    cr_assert(game.workspace.hasGameInput());
    game.click(areas.entities.left + (areas.entities.width / 2.0F), areas.entities.top + (areas.entities.height / 2.0F));
    cr_assert_not(game.workspace.hasGameInput());
}

Test(editor_workspace, editor_opens_again_without_the_inputs)
{
    skipWithoutDisplay();

    Game game;
    const EditorAreas areas = getEditorAreas(SIZE);

    game.press(ImGuiKey_F2);
    game.click(areas.preview.left + (areas.preview.width / 2.0F), areas.preview.top + (areas.preview.height / 2.0F));
    game.press(ImGuiKey_F2);
    cr_assert(game.workspace.hasGameInput());
    game.press(ImGuiKey_F2);
    cr_assert_not(game.workspace.hasGameInput());
}

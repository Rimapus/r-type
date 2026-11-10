/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/src/Workspace
** File description:
** Workspace source file
*/

#include "younity/editor/Workspace.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/render/DebugWindow.hpp"
#include <array>
#include <cstddef>
#include <imgui.h>
#include <optional>
#include <stdexcept>

namespace younity::editor {

namespace {

// The keys from Left to RAlt, in the order of input::Key
constexpr std::array<ImGuiKey, 15> OTHER_KEYS{
    ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow, ImGuiKey_Space,
    ImGuiKey_Enter, ImGuiKey_Escape, ImGuiKey_Backspace, ImGuiKey_Tab, ImGuiKey_LeftShift,
    ImGuiKey_RightShift, ImGuiKey_LeftCtrl, ImGuiKey_RightCtrl, ImGuiKey_LeftAlt, ImGuiKey_RightAlt};

static_assert(OTHER_KEYS.size() == static_cast<std::size_t>(input::Key::F1) - static_cast<std::size_t>(input::Key::Left));

/**
 * @brief Get the key of the debug interface for a key of the engine. The debug interface gets the keys from the
 *        events of the window, so a key pressed while another window has the focus is not seen.
 *
 * @param key The key of the engine
 *
 * @return The same key in the debug interface.
 */
ImGuiKey toImGuiKey(input::Key key)
{
    const auto index = static_cast<int>(key);

    if (key <= input::Key::Z) {
        return static_cast<ImGuiKey>(ImGuiKey_A + index);
    }
    if (key <= input::Key::Num9) {
        return static_cast<ImGuiKey>(ImGuiKey_0 + index - static_cast<int>(input::Key::Num0));
    }
    if (key >= input::Key::F1) {
        return static_cast<ImGuiKey>(ImGuiKey_F1 + index - static_cast<int>(input::Key::F1));
    }
    return OTHER_KEYS[static_cast<std::size_t>(index - static_cast<int>(input::Key::Left))];
}

/**
 * @brief Check if a key was pressed since the last frame, the key repeat of the system left out.
 *
 * @param key The key, if one was chosen
 *
 * @return true if the key was chosen and pressed.
 */
bool isPressed(const std::optional<input::Key> &key)
{
    return key && ImGui::IsKeyPressed(toImGuiKey(*key), false);
}

} // namespace

Workspace::Workspace(Editor &editor, ConsoleWindow &console, MetricsWindow &metrics, AssetBrowser &assets) noexcept
    : _editor(editor),
      _console(console),
      _metrics(metrics),
      _assets(assets) {}

void Workspace::bindKey(Layout layout, input::Key key)
{
    if (layout == Layout::Debug) {
        this->_debugKey = key;
    } else if (layout == Layout::Editor) {
        this->_editorKey = key;
    } else {
        throw std::invalid_argument("younity::editor::Workspace: the game layout has no key");
    }
}

void Workspace::toggle(Layout layout) noexcept
{
    this->_layout = this->_layout == layout ? Layout::Game : layout;
    // The editor layout opens without the inputs of the game: a click in the preview gives them
    this->_isPreviewFocused = false;
}

Layout Workspace::getLayout(void) const noexcept
{
    return this->_layout;
}

bool Workspace::hasGameInput(void) const
{
    if (this->_layout == Layout::Debug) {
        return !ImGui::GetIO().WantCaptureKeyboard;
    }
    if (this->_layout == Layout::Editor) {
        return this->_isPreviewFocused;
    }
    return true;
}

void Workspace::draw(ecs::Registry &registry, render::Window &window)
{
    if (isPressed(this->_debugKey)) {
        this->toggle(Layout::Debug);
    } else if (isPressed(this->_editorKey)) {
        this->toggle(Layout::Editor);
    }

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const Vector2f size{display.x, display.y};

    if (this->_layout == Layout::Debug) {
        const DebugAreas areas = getDebugAreas(size);

        window.setGameArea(std::nullopt);
        render::drawDebugWindow(areas.younity);
        this->_metrics.draw(areas.metrics);
        this->_console.draw(areas.console);
    } else if (this->_layout == Layout::Editor) {
        const EditorAreas areas = getEditorAreas(size);

        window.setGameArea(areas.preview);
        this->focusPreview(areas.preview);
        this->_editor.draw(registry, window, areas);
        this->_assets.draw(registry, window, areas.assets);
        this->_console.draw(areas.console);
        this->_metrics.draw(areas.metrics);
    } else {
        window.setGameArea(std::nullopt);
    }
}

void Workspace::focusPreview(const Rectf &preview)
{
    const ImGuiIO &io = ImGui::GetIO();

    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left) || !ImGui::IsMousePosValid()) {
        return;
    }
    // A click on a window of the editor belongs to the debug interface, which then wants the mouse
    this->_isPreviewFocused = !io.WantCaptureMouse && preview.contains(Vector2f{io.MousePos.x, io.MousePos.y});
}

} // namespace younity::editor

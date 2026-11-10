/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/src/Editor
** File description:
** Editor source file
*/

#include "younity/editor/Editor.hpp"
#include "younity/core/Color.hpp"
#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/ecs/FieldValue.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/render/DebugWindow.hpp"
#include "younity/render/Renderer.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <imgui.h>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace younity::editor {

namespace {

/**
 * @brief Grow the string edited by ImGui::InputText to the length of the text typed.
 *
 * @param data The state of the text field, its user data is the edited std::string
 *
 * @return 0, as ImGui expects.
 */
int resizeText(ImGuiInputTextCallbackData *data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto *text = static_cast<std::string *>(data->UserData);

        text->resize(static_cast<std::size_t>(data->BufTextLen));
        data->Buf = text->data();
    }
    return 0;
}

/**
 * @brief Draw a text field editing a string.
 *
 * @param label The label of the field
 * @param text The string to edit
 * @param flags More ImGui flags of the field, e.g. ImGuiInputTextFlags_EnterReturnsTrue
 *
 * @return true if the text was changed, or confirmed with Enter for ImGuiInputTextFlags_EnterReturnsTrue.
 */
bool drawText(const char *label, std::string &text, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
{
    return ImGui::InputText(
        label, text.data(), text.capacity() + 1, ImGuiInputTextFlags_CallbackResize | flags, resizeText, &text);
}

/**
 * @brief Draw a color picker editing a color.
 *
 * @param label The label of the field
 * @param color The color to edit
 *
 * @return true if the color was changed.
 */
bool drawColor(const char *label, Color &color)
{
    std::uint8_t *parts[] = {&color.r, &color.g, &color.b, &color.a};
    float values[] = {color.r / 255.0F, color.g / 255.0F, color.b / 255.0F, color.a / 255.0F};

    if (!ImGui::ColorEdit4(label, values)) {
        return false;
    }
    for (std::size_t part = 0; part < 4; ++part) {
        *parts[part] = static_cast<std::uint8_t>(std::lround(values[part] * 255.0F));
    }
    return true;
}

/**
 * @brief Draw a checkbox telling if the rectangle is set, and its four values when it is.
 *
 * @param label The label of the field
 * @param rect The rectangle to edit, std::nullopt when it is not set
 *
 * @return true if the rectangle was changed.
 */
bool drawRect(const char *label, std::optional<Recti> &rect)
{
    bool isSet = rect.has_value();
    bool changed = false;

    ImGui::PushID(label);
    if (ImGui::Checkbox("##set", &isSet)) {
        rect = isSet ? std::optional<Recti>(Recti{}) : std::nullopt;
        changed = true;
    }
    ImGui::SameLine();
    if (rect) {
        int values[] = {rect->left, rect->top, rect->width, rect->height};

        if (ImGui::DragInt4(label, values)) {
            *rect = Recti{values[0], values[1], values[2], values[3]};
            changed = true;
        }
    } else {
        ImGui::TextUnformatted(label);
    }
    ImGui::PopID();
    return changed;
}

/**
 * @brief Draw the widget editing a field, chosen by the type of its value.
 *
 * @param label The label of the field, its name
 * @param value The value to edit
 *
 * @return true if the value was changed.
 */
bool drawField(const char *label, ecs::FieldValue &value)
{
    return std::visit(
        [label](auto &field) {
            using Field = std::decay_t<decltype(field)>;

            if constexpr (std::is_same_v<Field, bool>) {
                return ImGui::Checkbox(label, &field);
            } else if constexpr (std::is_same_v<Field, int>) {
                return ImGui::DragInt(label, &field);
            } else if constexpr (std::is_same_v<Field, float>) {
                return ImGui::DragFloat(label, &field);
            } else if constexpr (std::is_same_v<Field, std::string>) {
                // Applied on Enter: a field like the path of a texture cannot take each letter typed on the way
                return drawText(label, field, ImGuiInputTextFlags_EnterReturnsTrue);
            } else if constexpr (std::is_same_v<Field, Vector2f>) {
                return ImGui::DragFloat2(label, &field.x);
            } else if constexpr (std::is_same_v<Field, Color>) {
                return drawColor(label, field);
            } else if constexpr (std::is_same_v<Field, std::optional<Recti>>) {
                return drawRect(label, field);
            } else {
                bool changed = false;
                std::size_t removed = field.size();

                ImGui::TextUnformatted(label);
                for (std::size_t index = 0; index < field.size(); ++index) {
                    int values[] = {field[index].left, field[index].top, field[index].width, field[index].height};

                    ImGui::PushID(static_cast<int>(index));
                    if (ImGui::DragInt4("##frame", values)) {
                        field[index] = Recti{values[0], values[1], values[2], values[3]};
                        changed = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove")) {
                        removed = index;
                        changed = true;
                    }
                    ImGui::PopID();
                }
                if (removed != field.size()) {
                    field.erase(field.begin() + static_cast<std::ptrdiff_t>(removed));
                }
                if (ImGui::SmallButton("Add frame")) {
                    field.push_back(Recti{});
                    changed = true;
                }
                return changed;
            }
        },
        value);
}

/**
 * @brief Format a scene file error for the Scene window.
 *
 * @param action "save" or "load"
 * @param error The error returned by the scene file functions
 *
 * @return The reason, with the file and the line when there is one.
 */
std::string sceneErrorText(std::string_view action, const ecs::SceneFileError &error)
{
    std::string text = "Could not " + std::string(action) + " " + error.path.string();

    if (error.line != 0) {
        text += " at line " + std::to_string(error.line);
    }
    text += ": " + error.message;
    return text;
}

} // namespace

Editor::Editor(const ecs::ComponentRegistry &components)
    : _components(components) {}

Editor::~Editor(void)
{
    if (this->_console != nullptr) {
        for (const std::string_view name : {"pause", "play", "step"}) {
            this->_console->removeCommand(name);
        }
    }
}

void Editor::registerCommands(tools::Console &console)
{
    console.addCommand("pause", "Freeze the game, like the Pause button of the editor",
                       [this](std::span<const std::string_view>) -> std::expected<std::string, tools::ConsoleError> {
                           this->pause();
                           return std::string("Paused");
                       });
    console.addCommand("play", "Resume the game, like the Play button of the editor",
                       [this](std::span<const std::string_view>) -> std::expected<std::string, tools::ConsoleError> {
                           this->play();
                           return std::string("Playing");
                       });
    console.addCommand("step", "Let one step of the paused game through, like the Step button of the editor",
                       [this](std::span<const std::string_view>) -> std::expected<std::string, tools::ConsoleError> {
                           if (!this->isPaused()) {
                               return std::unexpected(tools::ConsoleError{"step works while the game is paused"});
                           }
                           this->step();
                           return std::string("One step");
                       });
    this->_console = &console;
}

void Editor::draw(ecs::Registry &registry, const render::Window &window, const std::optional<EditorAreas> &areas)
{
    if (this->_selected && !registry.isAlive(*this->_selected)) {
        this->_selected.reset();
    }
    this->handleMouse(registry, window);
    this->drawOutline(registry, window);
    std::optional<Rectf> listArea = areas ? std::optional(areas->entities) : std::nullopt;

    // The entity list goes down over the place of the hidden Scene window, below it in the same column
    if (listArea && !this->_isSceneWindowShown) {
        listArea->height += areas->scene.height;
    }
    this->drawEntityList(registry, listArea);
    this->drawInspector(registry, areas ? std::optional(areas->inspector) : std::nullopt);
    if (this->_isSceneWindowShown) {
        this->drawSceneFile(registry, areas ? std::optional(areas->scene) : std::nullopt);
    }
}

void Editor::select(std::optional<ecs::Entity> entity) noexcept
{
    this->_selected = entity;
    this->_dragOffset.reset();
}

std::optional<ecs::Entity> Editor::getSelected(void) const noexcept
{
    return this->_selected;
}

void Editor::setSceneWindowShown(bool shown) noexcept
{
    this->_isSceneWindowShown = shown;
}

bool Editor::isSceneWindowShown(void) const noexcept
{
    return this->_isSceneWindowShown;
}

std::expected<void, ecs::SceneFileError> Editor::saveScene(const ecs::Registry &registry, const std::filesystem::path &path) const
{
    return ecs::saveScene(registry, this->_components, path);
}

std::expected<void, ecs::SceneFileError> Editor::loadScene(ecs::Registry &registry, const std::filesystem::path &path)
{
    const std::expected<void, ecs::SceneFileError> result = ecs::loadScene(registry, this->_components, path);

    if (!result.has_value()) {
        return result;
    }
    this->_selected.reset();
    return result;
}

void Editor::play(void) noexcept
{
    this->_isPaused = false;
    this->_pendingSteps = 0;
}

void Editor::pause(void) noexcept
{
    this->_isPaused = true;
}

void Editor::step(void) noexcept
{
    if (this->_isPaused) {
        ++this->_pendingSteps;
    }
}

bool Editor::isPaused(void) const noexcept
{
    return this->_isPaused;
}

bool Editor::shouldUpdate(void) noexcept
{
    if (!this->_isPaused) {
        return true;
    }
    if (this->_pendingSteps > 0) {
        --this->_pendingSteps;
        return true;
    }
    return false;
}

void Editor::handleMouse(ecs::Registry &registry, const render::Window &window)
{
    const ImGuiIO &io = ImGui::GetIO();

    if (!ImGui::IsMousePosValid()) {
        return;
    }

    const Vector2f cursor = window.mapPixelToWorld(Vector2i{static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y)});

    // A click on an editor window belongs to the window, e.g. a button or the entity list
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.WantCaptureMouse) {
        this->select(render::getEntityAt(registry, cursor));
        if (this->_selected) {
            this->_dragOffset = registry.getComponent<ecs::Transform>(*this->_selected).position - cursor;
        }
    }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) || !this->_selected ||
        !registry.hasComponent<ecs::Transform>(*this->_selected)) {
        this->_dragOffset.reset();
    }
    if (this->_dragOffset) {
        registry.getComponent<ecs::Transform>(*this->_selected).position = cursor + *this->_dragOffset;
    }
}

void Editor::drawOutline(const ecs::Registry &registry, const render::Window &window) const
{
    if (!this->_selected) {
        return;
    }
    for (const render::DrawItem &item : render::getDrawOrder(registry)) {
        if (item.entity != *this->_selected) {
            continue;
        }

        std::array<ImVec2, 4> points{};
        const std::array<Vector2f, 4> corners = render::getDrawCorners(registry, item);

        for (std::size_t corner = 0; corner < corners.size(); ++corner) {
            const Vector2i pixel = window.mapWorldToPixel(corners[corner]);

            points[corner] = ImVec2(static_cast<float>(pixel.x), static_cast<float>(pixel.y));
        }
        ImGui::GetBackgroundDrawList()->AddQuad(points[0], points[1], points[2], points[3], IM_COL32(255, 200, 0, 255), 2.0F);
    }
}

void Editor::drawPlayback(void)
{
    ImGui::BeginDisabled(!this->_isPaused);
    if (ImGui::Button("Play")) {
        this->play();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(this->_isPaused);
    if (ImGui::Button("Pause")) {
        this->pause();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!this->_isPaused);
    if (ImGui::Button("Step")) {
        this->step();
    }
    ImGui::EndDisabled();
    ImGui::Separator();
}

void Editor::drawEntityList(const ecs::Registry &registry, const std::optional<Rectf> &area)
{
    if (render::beginDebugWindow(ENTITY_LIST_TITLE, area)) {
        this->drawPlayback();
        for (const ecs::Entity entity : registry.getEntities()) {
            const std::string label = "Entity " + std::to_string(entity.getId());

            if (ImGui::Selectable(label.c_str(), this->_selected == entity)) {
                this->_selected = entity;
            }
        }
    }
    ImGui::End();
}

void Editor::drawInspector(ecs::Registry &registry, const std::optional<Rectf> &area)
{
    if (render::beginDebugWindow(INSPECTOR_TITLE, area)) {
        if (!this->_selected) {
            ImGui::TextUnformatted("Select an entity in the Entities window");
        } else {
            const ecs::Entity entity = *this->_selected;

            ImGui::Text("Entity %u", entity.getId());
            for (const std::string &name : this->_components.getNamesOf(registry, entity)) {
                const ecs::ComponentInfo &component = this->_components.get(name);

                // Two components may have a field of the same name, e.g. "value"
                ImGui::PushID(name.c_str());
                if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (const std::string &field : component.getFieldNames()) {
                        ecs::FieldValue value = component.getField(registry, entity, field);

                        if (!drawField(field.c_str(), value)) {
                            continue;
                        }
                        try {
                            component.setField(registry, entity, field, value);
                        } catch (const std::out_of_range &) {
                            // An integer dragged past the range of its field, e.g. 256 for a std::uint8_t: the field keeps its value
                        } catch (const std::invalid_argument &) {
                            // A value the field refuses, e.g. the path of a texture that cannot be loaded: the field keeps its value
                        }
                    }
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::End();
}

void Editor::drawSceneFile(ecs::Registry &registry, const std::optional<Rectf> &area)
{
    if (render::beginDebugWindow(SCENE_TITLE, area)) {
        if (drawText("File", this->_scenePath)) {
            this->_sceneStatus.clear();
        }
        const bool hasPath = !this->_scenePath.empty();

        ImGui::BeginDisabled(!hasPath);
        if (ImGui::Button("Save")) {
            const std::expected<void, ecs::SceneFileError> result = this->saveScene(registry, this->_scenePath);

            if (result.has_value()) {
                this->_sceneStatus = "Saved " + this->_scenePath;
                this->_sceneStatusIsError = false;
            } else {
                this->_sceneStatus = sceneErrorText("save", result.error());
                this->_sceneStatusIsError = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            const std::expected<void, ecs::SceneFileError> result = this->loadScene(registry, this->_scenePath);

            if (result.has_value()) {
                this->_sceneStatus = "Loaded " + this->_scenePath;
                this->_sceneStatusIsError = false;
            } else {
                this->_sceneStatus = sceneErrorText("load", result.error());
                this->_sceneStatusIsError = true;
            }
        }
        ImGui::EndDisabled();
        if (!this->_sceneStatus.empty()) {
            ImGui::PushTextWrapPos(0.0F);
            if (this->_sceneStatusIsError) {
                ImGui::TextColored(ImVec4(1.0F, 0.35F, 0.35F, 1.0F), "%s", this->_sceneStatus.c_str());
            } else {
                ImGui::TextUnformatted(this->_sceneStatus.c_str());
            }
            ImGui::PopTextWrapPos();
        }
    }
    ImGui::End();
}

} // namespace younity::editor

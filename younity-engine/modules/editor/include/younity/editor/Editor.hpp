/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/include/younity/editor/Editor
** File description:
** Editor header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <string>

#include "younity/core/Vector2.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/SceneFile.hpp"
#include "younity/editor/Layout.hpp"
#include "younity/render/Window.hpp"
#include "younity/tools/Console.hpp"

namespace younity::editor {

/**
 * @brief Title of the window listing the entities, also its id in the debug interface.
 */
inline constexpr const char *ENTITY_LIST_TITLE = "Entities";

/**
 * @brief Title of the window editing the components of the selected entity, also its id in the debug interface.
 */
inline constexpr const char *INSPECTOR_TITLE = "Inspector";

/**
 * @brief Title of the window saving and loading scene files, also its id in the debug interface.
 */
inline constexpr const char *SCENE_TITLE = "Scene";

/**
 * @brief The editor of the engine, drawn in the debug interface over the game: the list of the entities of a
 *        registry, an inspector to edit the components of the selected one, field by field, a window to
 *        save and load scene files, and the Play, Pause and Step buttons that control the steps of the game.
 *        In the game view, a click selects the entity under the mouse cursor and a drag moves it.
 */
class Editor
{
public:
    /**
     * @brief Create an editor, without any selected entity.
     *
     * @param components The component registry of the game: the inspector shows the components registered in it
     */
    explicit Editor(const ecs::ComponentRegistry &components);

    /**
     * @brief Remove the commands added with registerCommands(), so the console never calls a destroyed editor.
     */
    ~Editor(void);

    Editor(const Editor &) = delete;
    Editor &operator=(const Editor &) = delete;

    /**
     * @brief Add `pause`, `play` and `step` to a console, the Play, Pause and Step buttons as commands.
     *        They are removed when the editor is destroyed.
     *
     * @param console The console, e.g. tools::console(); it must outlive the editor
     *
     * @throw std::invalid_argument if one of the names is already used.
     */
    void registerCommands(tools::Console &console);

    /**
     * @brief Draw the entity list, with the Play, Pause and Step buttons on top, the inspector and the scene window.
     *        Called once per frame between window.clear() and window.display(). A click in the list selects an
     *        entity, a field changed in the inspector is written to its component, and the scene window saves or
     *        loads the registry.
     *        A click in the game view, outside the editor windows, selects the entity under the cursor, or clears
     *        the selection over an empty place. Holding the button down drags it: its Transform follows the cursor.
     *        The selected entity is outlined.
     *        A selected entity that is not alive anymore is unselected.
     *
     * @param registry The registry whose entities are shown, e.g. the one of the active scene
     * @param window The window the registry is drawn in, to bring the cursor back to the world
     * @param areas Where the entity list, the scene window and the inspector are fixed, e.g. by a Workspace, or nothing
     *        to let the mouse move and resize them
     */
    void draw(ecs::Registry &registry, const render::Window &window, const std::optional<EditorAreas> &areas = std::nullopt);

    /**
     * @brief Select the entity shown in the inspector. A drag in progress stops.
     *
     * @param entity The entity to select, or std::nullopt to clear the selection
     */
    void select(std::optional<ecs::Entity> entity) noexcept;

    /**
     * @brief Get the entity shown in the inspector.
     *
     * @return The selected entity, or std::nullopt when the selection is empty.
     */
    [[nodiscard]] std::optional<ecs::Entity> getSelected(void) const noexcept;

    /**
     * @brief Show or hide the Scene window, e.g. hidden in a networked client, whose world comes from the server and
     *        cannot be replaced by a scene file. Hidden, the entity list takes its place in the editor areas.
     *        saveScene() and loadScene() still work.
     *
     * @param shown true to draw the Scene window, the default
     */
    void setSceneWindowShown(bool shown) noexcept;

    /**
     * @brief Check if the Scene window is drawn.
     *
     * @return true unless setSceneWindowShown(false) hid it.
     */
    [[nodiscard]] bool isSceneWindowShown(void) const noexcept;

    /**
     * @brief Save the entities and components of a registry to a scene file.
     *
     * Only the component types registered in the ComponentRegistry are written; see ecs::saveScene.
     *
     * @param registry The registry owning the entities and components
     * @param path The file to write
     *
     * @return Nothing, or why the file could not be written.
     */
    [[nodiscard]] std::expected<void, ecs::SceneFileError> saveScene(
        const ecs::Registry &registry, const std::filesystem::path &path) const;

    /**
     * @brief Load a scene file into a registry and clear the selection.
     *
     * The file is checked in a temporary registry first, so an invalid file leaves the registry untouched.
     * On success the entities have the ids of the file and the registry keeps its systems; see ecs::loadScene.
     *
     * @param registry The registry to replace, outside of updateSystems()
     * @param path The file to read
     *
     * @return Nothing, or why the file could not be loaded.
     *
     * @throw std::logic_error if called during updateSystems(): the entities cannot be destroyed while a system runs.
     */
    [[nodiscard]] std::expected<void, ecs::SceneFileError> loadScene(ecs::Registry &registry, const std::filesystem::path &path);

    /**
     * @brief Resume the game: shouldUpdate() lets every step through.
     */
    void play(void) noexcept;

    /**
     * @brief Freeze the game: shouldUpdate() holds the steps back, except the ones asked with step().
     */
    void pause(void) noexcept;

    /**
     * @brief Let one more step through while the game is paused. Only acts while paused.
     */
    void step(void) noexcept;

    /**
     * @brief Check if the game is paused.
     *
     * @return true from pause() until play(), false at the creation.
     */
    [[nodiscard]] bool isPaused(void) const noexcept;

    /**
     * @brief Tell if the game moves forward at this fixed step, e.g. if (editor.shouldUpdate()) scenes.update(step).
     *        Called once per fixed step, in onFixedUpdate(): a step asked with step() is used by this call.
     *
     * @return true while playing, and once per step() asked while paused.
     */
    [[nodiscard]] bool shouldUpdate(void) noexcept;

private:
    /**
     * @brief Select the entity clicked in the game view and move the one dragged.
     *
     * @param registry The registry whose entities are clicked
     * @param window The window the registry is drawn in
     */
    void handleMouse(ecs::Registry &registry, const render::Window &window);

    /**
     * @brief Outline the Sprite and the Text of the selected entity, behind the editor windows.
     *
     * @param registry The registry owning the selected entity
     * @param window The window the registry is drawn in
     */
    void drawOutline(const ecs::Registry &registry, const render::Window &window) const;

    /**
     * @brief Draw the Play, Pause and Step buttons: Play and Step act while paused, Pause while playing.
     */
    void drawPlayback(void);

    /**
     * @brief Draw the window listing the entities of the registry, and select the one clicked.
     *
     * @param registry The registry whose entities are shown
     * @param area Where the window is fixed, or nothing to let the mouse move it
     */
    void drawEntityList(const ecs::Registry &registry, const std::optional<Rectf> &area);

    /**
     * @brief Draw the window editing the components of the selected entity.
     *
     * @param registry The registry owning the selected entity
     * @param area Where the window is fixed, or nothing to let the mouse move it
     */
    void drawInspector(ecs::Registry &registry, const std::optional<Rectf> &area);

    /**
     * @brief Draw the window saving and loading scene files.
     *
     * @param registry The registry to save, or to replace with a loaded file
     * @param area Where the window is fixed, or nothing to let the mouse move it
     */
    void drawSceneFile(ecs::Registry &registry, const std::optional<Rectf> &area);

    const ecs::ComponentRegistry &_components;
    std::optional<ecs::Entity> _selected;
    std::string _scenePath = "level.json";
    std::string _sceneStatus;
    bool _sceneStatusIsError = false;
    bool _isSceneWindowShown = true;
    // The console registerCommands() added the commands to, empty without them
    tools::Console *_console = nullptr;
    bool _isPaused = false;
    // Steps asked with step() and not used by shouldUpdate() yet: two clicks may come before the next fixed step
    unsigned int _pendingSteps = 0;
    // From the cursor to the position of the dragged entity, kept so the entity does not jump to the cursor. Empty without a drag
    std::optional<Vector2f> _dragOffset;
};

} // namespace younity::editor

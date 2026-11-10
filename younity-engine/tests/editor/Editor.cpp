/*
** EPITECH PROJECT, 2026
** younity-engine/tests/editor/Editor
** File description:
** Editor unit tests
*/

#include <criterion/criterion.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "../TestSupport.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Components.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/SceneFile.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/editor/Editor.hpp"
#include "younity/editor/Layout.hpp"
#include "younity/render/Components.hpp"
#include "younity/render/Texture.hpp"
#include "younity/render/Window.hpp"
#include "younity/render/components/Animation.hpp"
#include "younity/render/components/Sprite.hpp"

using younity::Rectf;
using younity::Recti;
using younity::Vector2f;
using younity::assets::ResourceManager;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::editor::Editor;
using younity::editor::EditorAreas;
using younity::editor::ENTITY_LIST_TITLE;
using younity::editor::getEditorAreas;
using younity::editor::INSPECTOR_TITLE;
using younity::editor::SCENE_TITLE;
using younity::render::Sprite;
using younity::render::Texture;
using younity::render::Window;
using younity::test::isDrawn;
using younity::test::isFixedAt;
using younity::test::skipWithoutDisplay;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "render" / "fixtures";

// Draw one frame of the editor, in the debug interface of the window.
// window.clear() is skipped: it would read the real mouse over the one moved by the test
void drawFrame(Editor &editor, Registry &registry, const Window &window)
{
    ImGui::NewFrame();
    editor.draw(registry, window);
    ImGui::Render();
}

// Move the mouse cursor and press or release its left button for the next frame
void setMouse(float x, float y, bool isDown)
{
    ImGuiIO &io = ImGui::GetIO();

    io.AddMousePosEvent(x, y);
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, isDown);
}

// A sprite of the 3 by 2 red image, 30 by 20 pixels from its position, away from the editor windows
Entity addShip(Registry &registry)
{
    auto texture = Texture::loadFromFile(FIXTURES / "red_3x2.png");
    const Entity entity = registry.createEntity();
    Sprite sprite;

    cr_assert(texture.has_value());
    sprite.texture = std::make_shared<const Texture>(std::move(*texture));
    registry.addComponent(entity, Transform{
                                      .position = {600.0F, 400.0F},
                                      .scale = {10.0F,  10.0F }
    });
    registry.addComponent(entity, sprite);
    return entity;
}

class TempFile
{
public:
    explicit TempFile(std::string_view name)
        : _path(std::filesystem::temp_directory_path() / makeName(name)) {}

    ~TempFile(void)
    {
        std::error_code error;

        std::filesystem::remove(this->_path, error);
    }

    TempFile(const TempFile &) = delete;
    TempFile &operator=(const TempFile &) = delete;

    [[nodiscard]] const std::filesystem::path &getPath(void) const noexcept
    {
        return this->_path;
    }

private:
    [[nodiscard]] static std::string makeName(std::string_view name)
    {
        static std::random_device random;

        return "younity-editor-scene-" + std::to_string(random()) + "-" + std::to_string(random()) + "-" +
               std::string(name) + ".json";
    }

    std::filesystem::path _path;
};

} // namespace

Test(editor_editor, draws_the_entity_list_and_the_inspector)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);

    drawFrame(editor, registry, window);
    cr_assert(isDrawn(ENTITY_LIST_TITLE));
    cr_assert(isDrawn(INSPECTOR_TITLE));
}

Test(editor_editor, fixes_its_windows_in_the_editor_areas)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const EditorAreas areas = getEditorAreas(Vector2f{800.0F, 600.0F});

    ImGui::NewFrame();
    editor.draw(registry, window, areas);
    ImGui::Render();
    cr_assert(isFixedAt(ENTITY_LIST_TITLE, areas.entities));
    cr_assert(isFixedAt(INSPECTOR_TITLE, areas.inspector));
    cr_assert(isFixedAt(SCENE_TITLE, areas.scene));
}

Test(editor_editor, a_hidden_scene_window_gives_its_place_to_the_entity_list)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const EditorAreas areas = getEditorAreas(Vector2f{800.0F, 600.0F});
    const Rectf column{areas.entities.left, areas.entities.top, areas.entities.width,
                       areas.entities.height + areas.scene.height};

    editor.setSceneWindowShown(false);
    ImGui::NewFrame();
    editor.draw(registry, window, areas);
    ImGui::Render();
    cr_assert_not(editor.isSceneWindowShown());
    cr_assert_not(isDrawn(SCENE_TITLE));
    cr_assert(isFixedAt(ENTITY_LIST_TITLE, column));
}

Test(editor_editor, its_commands_pause_play_and_step_the_game)
{
    ComponentRegistry components;
    younity::tools::Console console;

    {
        Editor editor(components);

        editor.registerCommands(console);
        cr_assert_not(console.execute("step").has_value());
        cr_assert(console.execute("pause").has_value());
        cr_assert(editor.isPaused());
        cr_assert(console.execute("step").has_value());
        cr_assert(editor.shouldUpdate());
        cr_assert(console.execute("play").has_value());
        cr_assert_not(editor.isPaused());
    }
    cr_assert_not(console.hasCommand("pause"));
}

Test(editor_editor, starts_without_a_selection)
{
    const ComponentRegistry components;
    const Editor editor(components);

    cr_assert_not(editor.getSelected().has_value());
}

Test(editor_editor, select_an_entity)
{
    const ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();

    editor.select(entity);
    cr_assert(editor.getSelected() == entity);
    editor.select(std::nullopt);
    cr_assert_not(editor.getSelected().has_value());
}

Test(editor_editor, draws_the_inspector_of_the_selected_entity)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();

    younity::ecs::registerComponents(components);
    registry.addComponent(entity, Transform{});
    editor.select(entity);
    drawFrame(editor, registry, window);
    cr_assert(editor.getSelected() == entity);
    cr_assert(isDrawn(INSPECTOR_TITLE));
}

Test(editor_editor, draws_the_frames_of_an_animation)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    ResourceManager resources;
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();

    younity::ecs::registerComponents(components);
    younity::render::registerComponents(components, resources);
    younity::render::Animation animation;

    animation.clip.frames = {
        Recti{0,  0, 16, 16},
        Recti{16, 0, 16, 16}
    };
    animation.clip.frameDuration = 0.25F;
    animation.clip.loop = false;
    registry.addComponent(entity, animation);
    editor.select(entity);
    drawFrame(editor, registry, window);
    cr_assert(isDrawn(INSPECTOR_TITLE));
}

Test(editor_editor, destroyed_entity_is_unselected)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();

    editor.select(entity);
    registry.destroyEntity(entity);
    drawFrame(editor, registry, window);
    cr_assert_not(editor.getSelected().has_value());
}

Test(editor_editor, alive_entity_stays_selected)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();

    editor.select(entity);
    drawFrame(editor, registry, window);
    drawFrame(editor, registry, window);
    cr_assert(editor.getSelected() == entity);
}

Test(editor_editor, draws_the_scene_window)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);

    drawFrame(editor, registry, window);
    cr_assert(editor.isSceneWindowShown());
    cr_assert(isDrawn(SCENE_TITLE));
}

Test(editor_editor, saves_the_registry_to_a_scene_file)
{
    ComponentRegistry components;
    ResourceManager resources;
    younity::ecs::registerComponents(components);
    younity::render::registerComponents(components, resources);
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();
    const TempFile file("save");
    const Transform transform{
        Vector2f{3.0F, 4.0F}
    };

    registry.addComponent(entity, transform);
    younity::render::Animation animation;

    animation.clip.frames = {
        Recti{0,  0, 16, 16},
        Recti{16, 0, 16, 16}
    };
    animation.clip.frameDuration = 0.25F;
    animation.clip.loop = false;
    animation.frame = 1;
    registry.addComponent(entity, animation);
    const auto saved = editor.saveScene(registry, file.getPath());
    cr_assert(saved.has_value());

    Registry loaded;
    const auto result = younity::ecs::loadScene(loaded, components, file.getPath());
    cr_assert(result.has_value());
    const std::vector<Entity> entities = loaded.getEntities();

    cr_assert_eq(entities.size(), 1);
    const Entity loadedEntity = entities.front();

    cr_assert(loaded.hasComponent<Transform>(loadedEntity));
    cr_assert(loaded.hasComponent<younity::render::Animation>(loadedEntity));
    const Transform &loadedTransform = loaded.getComponent<Transform>(loadedEntity);
    const younity::render::Animation &loadedAnimation = loaded.getComponent<younity::render::Animation>(loadedEntity);

    cr_assert_float_eq(loadedTransform.position.x, 3.0F, 0.0001F);
    cr_assert_float_eq(loadedTransform.position.y, 4.0F, 0.0001F);
    cr_assert(loadedAnimation.clip.frames == (std::vector<Recti>{
                                                 Recti{0,  0, 16, 16},
                                                 Recti{16, 0, 16, 16}
    }));
    cr_assert_float_eq(loadedAnimation.clip.frameDuration, 0.25F, 0.0001F);
    cr_assert_not(loadedAnimation.clip.loop);
    cr_assert_eq(loadedAnimation.frame, 1U);
}

Test(editor_editor, loads_a_scene_and_clears_the_selection)
{
    ComponentRegistry components;
    younity::ecs::registerComponents(components);
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();
    const TempFile file("load");

    registry.addComponent(entity, Transform{});
    const auto saved = editor.saveScene(registry, file.getPath());
    cr_assert(saved.has_value());
    editor.select(entity);
    const std::expected<void, younity::ecs::SceneFileError> result = editor.loadScene(registry, file.getPath());

    cr_assert(result.has_value());
    cr_assert_not(editor.getSelected().has_value());
    cr_assert(registry.getEntities() == std::vector<Entity>{entity});
}

Test(editor_editor, save_then_load_gives_back_the_same_picture)
{
    skipWithoutDisplay();

    ComponentRegistry components;
    ResourceManager resources(FIXTURES);
    Registry registry;
    Editor editor(components);
    const TempFile file("picture");

    resources.setLoader<Texture>(Texture::loadFromFile);
    younity::ecs::registerComponents(components);
    younity::render::registerComponents(components, resources);
    (void)registry.createEntity();
    const Entity ship = registry.createEntity();

    registry.addComponent(ship, Transform{});
    Sprite sprite;

    sprite.texture = *resources.load<Texture>("red_3x2.png");
    registry.addComponent(ship, sprite);
    cr_assert(editor.saveScene(registry, file.getPath()).has_value());
    registry.getComponent<Sprite>(ship).texture.reset();
    cr_assert(editor.loadScene(registry, file.getPath()).has_value());
    cr_assert(registry.hasComponent<Sprite>(ship));
    cr_assert_eq(registry.getComponent<Sprite>(ship).texture, *resources.load<Texture>("red_3x2.png"));
}

Test(editor_editor, a_failed_load_keeps_the_registry_and_selection)
{
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();
    const TempFile file("missing");

    editor.select(entity);
    const std::expected<void, younity::ecs::SceneFileError> result = editor.loadScene(registry, file.getPath());

    cr_assert_not(result.has_value());
    cr_assert(result.error().path == file.getPath());
    cr_assert(registry.isAlive(entity));
    cr_assert(editor.getSelected() == entity);
}

Test(editor_editor, a_failed_save_returns_an_error)
{
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const TempFile parent("save-error");
    const std::filesystem::path path = parent.getPath() / "scene.json";
    const std::expected<void, younity::ecs::SceneFileError> result = editor.saveScene(registry, path);

    cr_assert_not(result.has_value());
    cr_assert(result.error().path == path);
}

Test(editor_editor, starts_playing)
{
    const ComponentRegistry components;
    Editor editor(components);

    cr_assert_not(editor.isPaused());
    cr_assert(editor.shouldUpdate());
}

Test(editor_editor, pause_holds_the_steps_back)
{
    const ComponentRegistry components;
    Editor editor(components);

    editor.pause();
    cr_assert(editor.isPaused());
    cr_assert_not(editor.shouldUpdate());
    cr_assert_not(editor.shouldUpdate());
}

Test(editor_editor, step_lets_one_step_through)
{
    const ComponentRegistry components;
    Editor editor(components);

    editor.pause();
    editor.step();
    cr_assert(editor.shouldUpdate());
    cr_assert_not(editor.shouldUpdate());
}

Test(editor_editor, each_step_lets_one_step_through)
{
    const ComponentRegistry components;
    Editor editor(components);

    editor.pause();
    editor.step();
    editor.step();
    cr_assert(editor.shouldUpdate());
    cr_assert(editor.shouldUpdate());
    cr_assert_not(editor.shouldUpdate());
}

Test(editor_editor, step_while_playing_is_ignored)
{
    const ComponentRegistry components;
    Editor editor(components);

    editor.step();
    editor.pause();
    cr_assert_not(editor.shouldUpdate());
}

Test(editor_editor, play_resumes_the_steps)
{
    const ComponentRegistry components;
    Editor editor(components);

    editor.pause();
    editor.step();
    editor.play();
    cr_assert_not(editor.isPaused());
    cr_assert(editor.shouldUpdate());
    cr_assert(editor.shouldUpdate());
}

Test(editor_editor, play_drops_the_steps_asked_while_paused)
{
    const ComponentRegistry components;
    Editor editor(components);

    editor.pause();
    editor.step();
    editor.play();
    editor.pause();
    cr_assert_not(editor.shouldUpdate());
}

Test(editor_editor, draws_while_paused)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);

    editor.pause();
    drawFrame(editor, registry, window);
    cr_assert(isDrawn(ENTITY_LIST_TITLE));
    cr_assert(isDrawn(INSPECTOR_TITLE));
}

Test(editor_editor, click_selects_the_entity_under_the_cursor)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity ship = addShip(registry);

    setMouse(610.0F, 405.0F, true);
    drawFrame(editor, registry, window);
    cr_assert(editor.getSelected() == ship);
}

Test(editor_editor, click_on_an_empty_place_clears_the_selection)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);

    editor.select(addShip(registry));
    setMouse(780.0F, 580.0F, true);
    drawFrame(editor, registry, window);
    cr_assert_not(editor.getSelected().has_value());
}

Test(editor_editor, drag_moves_the_entity_until_the_release)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity ship = addShip(registry);

    setMouse(610.0F, 405.0F, true);
    drawFrame(editor, registry, window);
    setMouse(650.0F, 425.0F, true);
    drawFrame(editor, registry, window);
    setMouse(650.0F, 425.0F, false);
    drawFrame(editor, registry, window);
    setMouse(700.0F, 500.0F, false);
    drawFrame(editor, registry, window);

    // The ship keeps the place of the cursor on it: it moved by 40 by 20, like the cursor during the drag
    const Transform &transform = registry.getComponent<Transform>(ship);
    cr_assert_float_eq(transform.position.x, 640.0F, 0.0001F);
    cr_assert_float_eq(transform.position.y, 420.0F, 0.0001F);
}

Test(editor_editor, click_on_an_editor_window_keeps_the_selection)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity entity = registry.createEntity();

    editor.select(entity);
    // A new window shows from its second frame, once its size is known
    drawFrame(editor, registry, window);
    drawFrame(editor, registry, window);

    const ImGuiWindow *list = ImGui::FindWindowByName(ENTITY_LIST_TITLE);

    cr_assert_not_null(list);
    setMouse(list->Pos.x + (list->Size.x / 2.0F), list->Pos.y + 5.0F, true);
    drawFrame(editor, registry, window);
    cr_assert(editor.getSelected() == entity);
}

Test(editor_editor, outlines_the_selected_entity)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Editor");
    ComponentRegistry components;
    Registry registry;
    Editor editor(components);
    const Entity ship = addShip(registry);

    drawFrame(editor, registry, window);
    cr_assert_eq(ImGui::GetBackgroundDrawList()->VtxBuffer.Size, 0);
    editor.select(ship);
    drawFrame(editor, registry, window);
    cr_assert_gt(ImGui::GetBackgroundDrawList()->VtxBuffer.Size, 0);
}

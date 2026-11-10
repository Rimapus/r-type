/*
** EPITECH PROJECT, 2026
** younity-engine/tests/editor/AssetBrowser
** File description:
** AssetBrowser unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include <vector>

#include "../TestSupport.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/editor/AssetBrowser.hpp"
#include "younity/render/Texture.hpp"
#include "younity/render/Window.hpp"
#include "younity/render/components/Sprite.hpp"

using younity::Rectf;
using younity::Vector2f;
using younity::assets::ResourceManager;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::editor::Asset;
using younity::editor::ASSET_BROWSER_TITLE;
using younity::editor::AssetBrowser;
using younity::editor::AssetKind;
using younity::render::Sprite;
using younity::render::Texture;
using younity::render::Window;
using younity::test::isFixedAt;
using younity::test::skipWithoutDisplay;

namespace {

const std::filesystem::path IMAGE = std::filesystem::path(YOUNITY_TESTS_DIR) / "render" / "fixtures" / "red_3x2.png";

// Criterion runs the tests in parallel: each test uses its own folder, removed before it
std::filesystem::path testFolder(const char *test)
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / ("younity_assets_" + std::string(test));

    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder / "sounds");
    std::filesystem::copy_file(IMAGE, folder / "ship.png");
    std::ofstream(folder / "sounds" / "shot.WAV") << "not read";
    std::ofstream(folder / "readme.txt") << "not an asset";
    return folder;
}

// A mouse move and a click in the same frame are both seen in this frame
void disableTrickling(void)
{
    ImGui::GetIO().ConfigInputTrickleEventQueue = false;
}

// Draw one frame of the browser, with the mouse at a position and its left button down or up
// window.clear() is skipped: it would read the real mouse over the one moved by the test
void drawFrame(AssetBrowser &browser, Registry &registry, const Window &window, ImVec2 mouse, bool down)
{
    ImGuiIO &io = ImGui::GetIO();

    io.AddMousePosEvent(mouse.x, mouse.y);
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, down);
    ImGui::NewFrame();
    browser.draw(registry, window);
    ImGui::Render();
}

} // namespace

Test(editor_asset_browser, lists_the_textures_and_the_sounds)
{
    ResourceManager resources(testFolder("listed"));
    const AssetBrowser browser(resources);

    cr_assert(browser.getAssets() == (std::vector<Asset>{
                                         {"ship.png",                                   AssetKind::Texture},
                                         {std::filesystem::path("sounds") / "shot.WAV", AssetKind::Sound  },
    }));
}

Test(editor_asset_browser, refresh_lists_the_new_files)
{
    const std::filesystem::path folder = testFolder("refreshed");
    ResourceManager resources(folder);
    AssetBrowser browser(resources);

    std::filesystem::copy_file(IMAGE, folder / "enemy.png");
    browser.refresh();
    cr_assert_eq(browser.getAssets().size(), 3U);
    cr_assert(browser.getAssets()[0].path == "enemy.png");
}

Test(editor_asset_browser, missing_folder_lists_nothing)
{
    ResourceManager resources(std::filesystem::temp_directory_path() / "younity_assets_missing_folder");
    const AssetBrowser browser(resources);

    cr_assert(browser.getAssets().empty());
}

Test(editor_asset_browser, dropped_texture_creates_a_sprite)
{
    ResourceManager resources(testFolder("dropped"));
    AssetBrowser browser(resources);
    Registry registry;

    resources.setLoader<Texture>(Texture::loadFromFile);

    const std::optional<Entity> entity = browser.drop(registry, {"ship.png", AssetKind::Texture}, Vector2f{40.0F, 50.0F});

    cr_assert(entity.has_value());
    cr_assert(registry.getComponent<Transform>(*entity).position == (Vector2f{40.0F, 50.0F}));
    cr_assert(registry.getComponent<Sprite>(*entity).texture != nullptr);
    cr_assert(registry.getComponent<Sprite>(*entity).origin == (Vector2f{1.5F, 1.0F}));
}

Test(editor_asset_browser, dropped_sound_creates_nothing)
{
    ResourceManager resources(testFolder("sound"));
    AssetBrowser browser(resources);
    Registry registry;

    cr_assert_not(browser.drop(registry, {"sounds/shot.WAV", AssetKind::Sound}, Vector2f{}).has_value());
    cr_assert(registry.getEntities().empty());
}

Test(editor_asset_browser, texture_that_cannot_load_creates_nothing)
{
    ResourceManager resources(testFolder("unloadable"));
    AssetBrowser browser(resources);
    Registry registry;

    resources.setLoader<Texture>(Texture::loadFromFile);
    cr_assert_not(browser.drop(registry, {"missing.png", AssetKind::Texture}, Vector2f{}).has_value());
    cr_assert(registry.getEntities().empty());
}

Test(editor_asset_browser, draws_its_window)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Assets");

    disableTrickling();
    ResourceManager resources(testFolder("drawn"));
    AssetBrowser browser(resources);
    Registry registry;

    drawFrame(browser, registry, window, ImVec2(-1.0F, -1.0F), false);

    const ImGuiWindow *list = ImGui::FindWindowByName(ASSET_BROWSER_TITLE);

    cr_assert(list != nullptr && list->Active);
}

Test(editor_asset_browser, texture_dragged_on_the_game_creates_a_sprite)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Assets");

    disableTrickling();
    ResourceManager resources(testFolder("dropped_on_game"));
    AssetBrowser browser(resources);
    Registry registry;

    resources.setLoader<Texture>(Texture::loadFromFile);
    // A new window is sized during its first frames: then the place of the ship.png row is known
    for (int frame = 0; frame < 3; ++frame) {
        drawFrame(browser, registry, window, ImVec2(-1.0F, -1.0F), false);
    }

    const ImGuiWindow *list = ImGui::FindWindowByName(ASSET_BROWSER_TITLE);
    const ImVec2 row(list->Pos.x + 20.0F, list->DC.CursorStartPos.y + (ImGui::GetFrameHeightWithSpacing() * 1.5F));

    drawFrame(browser, registry, window, row, true);
    drawFrame(browser, registry, window, ImVec2(row.x + 10.0F, row.y + 10.0F), true);
    drawFrame(browser, registry, window, ImVec2(700.0F, 500.0F), true);
    drawFrame(browser, registry, window, ImVec2(700.0F, 500.0F), false);

    const std::vector<Entity> sprites = registry.getEntitiesWith<Sprite>();

    cr_assert_eq(sprites.size(), 1U);
    cr_assert(registry.getComponent<Transform>(sprites[0]).position == (Vector2f{700.0F, 500.0F}));
}

Test(editor_asset_browser, texture_dropped_on_a_window_creates_nothing)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Assets");

    disableTrickling();
    ResourceManager resources(testFolder("dropped_on_window"));
    AssetBrowser browser(resources);
    Registry registry;

    resources.setLoader<Texture>(Texture::loadFromFile);
    // A new window is sized during its first frames: then the place of the ship.png row is known
    for (int frame = 0; frame < 3; ++frame) {
        drawFrame(browser, registry, window, ImVec2(-1.0F, -1.0F), false);
    }

    const ImGuiWindow *list = ImGui::FindWindowByName(ASSET_BROWSER_TITLE);
    const ImVec2 row(list->Pos.x + 20.0F, list->DC.CursorStartPos.y + (ImGui::GetFrameHeightWithSpacing() * 1.5F));

    drawFrame(browser, registry, window, row, true);
    drawFrame(browser, registry, window, ImVec2(row.x + 10.0F, row.y + 10.0F), true);
    drawFrame(browser, registry, window, ImVec2(row.x + 10.0F, row.y + 10.0F), false);
    cr_assert(registry.getEntities().empty());
}

Test(editor_asset_browser, texture_dropped_on_another_window_creates_nothing)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Assets");

    disableTrickling();
    ResourceManager resources(testFolder("other_window"));
    AssetBrowser browser(resources);
    Registry registry;
    const auto drawWithOther = [&browser, &registry, &window](ImVec2 mouse, bool down) {
        ImGuiIO &io = ImGui::GetIO();

        io.AddMousePosEvent(mouse.x, mouse.y);
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, down);
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(600.0F, 400.0F));
        ImGui::SetNextWindowSize(ImVec2(150.0F, 150.0F));
        ImGui::Begin("Other");
        ImGui::End();
        browser.draw(registry, window);
        ImGui::Render();
    };

    resources.setLoader<Texture>(Texture::loadFromFile);
    for (int frame = 0; frame < 3; ++frame) {
        drawWithOther(ImVec2(-1.0F, -1.0F), false);
    }

    const ImGuiWindow *list = ImGui::FindWindowByName(ASSET_BROWSER_TITLE);
    const ImVec2 row(list->Pos.x + 20.0F, list->DC.CursorStartPos.y + (ImGui::GetFrameHeightWithSpacing() * 1.5F));

    drawWithOther(row, true);
    drawWithOther(ImVec2(row.x + 10.0F, row.y + 10.0F), true);
    drawWithOther(ImVec2(670.0F, 470.0F), true);
    drawWithOther(ImVec2(670.0F, 470.0F), false);
    cr_assert(registry.getEntities().empty());
}

Test(editor_asset_browser, is_fixed_in_its_area)
{
    skipWithoutDisplay();

    const Window window(800, 600, "Asset browser");
    ResourceManager resources(testFolder("fixed"));
    AssetBrowser browser(resources);
    Registry registry;

    ImGui::NewFrame();
    browser.draw(registry, window, Rectf{400.0F, 400.0F, 200.0F, 200.0F});
    ImGui::Render();
    cr_assert(isFixedAt(ASSET_BROWSER_TITLE, Rectf{400.0F, 400.0F, 200.0F, 200.0F}));
}

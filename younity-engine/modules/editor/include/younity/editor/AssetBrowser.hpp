/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/include/younity/editor/AssetBrowser
** File description:
** AssetBrowser header file
*/

#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "younity/assets/ResourceManager.hpp"
#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/render/Window.hpp"

namespace younity::editor {

/**
 * @brief Title of the window listing the assets, also its id in the debug interface.
 */
inline constexpr const char *ASSET_BROWSER_TITLE = "Assets";

/**
 * @brief Name of the drag and drop payload of a texture in the debug interface: its path, a C string.
 */
inline constexpr const char *ASSET_PAYLOAD = "YOUNITY_ASSET";

/**
 * @brief What an asset file holds, told by its extension.
 */
enum class AssetKind {
    /** @brief An image: .png, .jpg, .jpeg, .bmp, .tga or .gif. */
    Texture,
    /** @brief A sound: .wav, .ogg, .flac or .mp3. */
    Sound,
};

/**
 * @brief An asset file found in the asset folder.
 */
struct Asset
{
    // Relative to the asset folder, the path given to ResourceManager::load()
    std::filesystem::path path;
    AssetKind kind;

    bool operator==(const Asset &other) const = default;
};

/**
 * @brief The window of the editor listing the textures and the sounds of the asset folder.
 *        A texture dragged from the list and dropped on the game, outside the windows of the debug interface,
 *        creates an entity with a Transform at the mouse and a Sprite of the texture.
 */
class AssetBrowser
{
public:
    /**
     * @brief Create the browser and list the asset folder.
     *
     * @param resources The resource manager of the game: its folder is listed and the dropped textures go through it,
     *        it must outlive the browser and have a loader for render::Texture
     */
    explicit AssetBrowser(assets::ResourceManager &resources);

    /**
     * @brief List the asset folder again, e.g. after a file was added. The Refresh button of the window calls it.
     *        A folder that cannot be read lists nothing.
     */
    void refresh(void);

    /**
     * @brief Get the textures and the sounds found in the asset folder by the last refresh().
     *
     * @return The assets, sorted by path.
     */
    [[nodiscard]] const std::vector<Asset> &getAssets(void) const noexcept;

    /**
     * @brief Create the entity of an asset dropped on the game: for a texture, an entity with a Transform at the
     *        position and a Sprite of the texture, its origin at the center of the image.
     *
     * @param registry The registry the entity is created in
     * @param asset The dropped asset
     * @param position Where it was dropped, in the world of the entities
     *
     * @return The created entity, or std::nullopt for a sound, which no component holds, or a texture that cannot be loaded.
     */
    std::optional<ecs::Entity> drop(ecs::Registry &registry, const Asset &asset, Vector2f position);

    /**
     * @brief Draw the window listing the assets, with a Refresh button, and create the entity of a texture dropped on
     *        the game. Called once per frame between window.clear() and window.display().
     *
     * @param registry The registry the dropped textures create their entity in, e.g. the one of the active scene
     * @param window The window of the game, to place the entity under the mouse with Window::mapPixelToWorld()
     * @param area Where the window is fixed, in pixels from the top left corner of the window, e.g. by a Workspace, or
     *        nothing to let the mouse move and resize it
     */
    void draw(ecs::Registry &registry, const render::Window &window, const std::optional<Rectf> &area = std::nullopt);

private:
    assets::ResourceManager &_resources;
    std::vector<Asset> _assets;
};

} // namespace younity::editor

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/src/AssetBrowser
** File description:
** AssetBrowser source file
*/

#include "younity/editor/AssetBrowser.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/editor/Log.hpp"
#include "younity/render/DebugWindow.hpp"
#include "younity/render/Texture.hpp"
#include "younity/render/components/Sprite.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <imgui.h>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace younity::editor {

namespace {

constexpr std::array<std::string_view, 6> TEXTURE_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif"};
constexpr std::array<std::string_view, 4> SOUND_EXTENSIONS = {".wav", ".ogg", ".flac", ".mp3"};

/**
 * @brief Tell what a file holds from its extension, whatever its case.
 *
 * @param path The file
 *
 * @return The kind of asset, or std::nullopt for a file that is neither a texture nor a sound.
 */
std::optional<AssetKind> kindOf(const std::filesystem::path &path)
{
    std::string extension = path.extension().string();

    std::ranges::transform(extension, extension.begin(), [](unsigned char letter) { return std::tolower(letter); });
    if (std::ranges::find(TEXTURE_EXTENSIONS, extension) != TEXTURE_EXTENSIONS.end()) {
        return AssetKind::Texture;
    }
    if (std::ranges::find(SOUND_EXTENSIONS, extension) != SOUND_EXTENSIONS.end()) {
        return AssetKind::Sound;
    }
    return std::nullopt;
}

} // namespace

AssetBrowser::AssetBrowser(assets::ResourceManager &resources)
    : _resources(resources)
{
    this->refresh();
}

void AssetBrowser::refresh(void)
{
    const std::filesystem::path root = this->_resources.getRoot().empty() ? "." : this->_resources.getRoot();
    std::error_code error;
    // A folder that cannot be read is skipped, the others are still listed
    std::filesystem::recursive_directory_iterator file(root, std::filesystem::directory_options::skip_permission_denied, error);

    this->_assets.clear();
    // The error_code overloads: a folder removed while it is listed ends the list instead of throwing
    for (; !error && file != std::filesystem::recursive_directory_iterator(); file.increment(error)) {
        const std::optional<AssetKind> kind = file->is_regular_file(error) ? kindOf(file->path()) : std::nullopt;

        if (kind) {
            this->_assets.push_back(Asset{file->path().lexically_relative(root), *kind});
        }
    }
    if (error) {
        logger().warn("Cannot list the assets of {}: {}", root.generic_string(), error.message());
    }
    std::ranges::sort(this->_assets, [](const Asset &first, const Asset &second) { return first.path < second.path; });
}

const std::vector<Asset> &AssetBrowser::getAssets(void) const noexcept
{
    return this->_assets;
}

std::optional<ecs::Entity> AssetBrowser::drop(ecs::Registry &registry, const Asset &asset, Vector2f position)
{
    if (asset.kind != AssetKind::Texture) {
        return std::nullopt;
    }
    const auto texture = this->_resources.load<render::Texture>(asset.path);

    if (!texture) {
        return std::nullopt;
    }
    const Vector2<unsigned int> size = (*texture)->getSize();
    const ecs::Entity entity = registry.createEntity();
    render::Sprite sprite;

    sprite.texture = *texture;
    sprite.origin = {static_cast<float>(size.x) / 2.0F, static_cast<float>(size.y) / 2.0F};
    registry.addComponent(entity, ecs::Transform{position});
    registry.addComponent(entity, sprite);
    return entity;
}

void AssetBrowser::draw(ecs::Registry &registry, const render::Window &window, const std::optional<Rectf> &area)
{
    if (render::beginDebugWindow(ASSET_BROWSER_TITLE, area)) {
        if (ImGui::Button("Refresh")) {
            this->refresh();
        }
        for (std::size_t index = 0; index < this->_assets.size(); ++index) {
            const Asset &asset = this->_assets[index];
            const std::string path = asset.path.generic_string();
            const std::string label = (asset.kind == AssetKind::Texture ? "[texture] " : "[sound] ") + path;

            // The index is the id: a file name may hold the "##" of the ImGui ids
            ImGui::PushID(static_cast<int>(index));
            ImGui::Selectable(label.c_str());
            if (asset.kind == AssetKind::Texture && ImGui::BeginDragDropSource()) {
                // The path, not the index: the list may be refreshed during the drag
                ImGui::SetDragDropPayload(ASSET_PAYLOAD, path.c_str(), path.size() + 1);
                ImGui::TextUnformatted(label.c_str());
                ImGui::EndDragDropSource();
            }
            ImGui::PopID();
        }
    }
    ImGui::End();

    // Dropped on the game: the mouse is released over no window of the debug interface
    const ImGuiPayload *payload = ImGui::GetDragDropPayload();

    // The dragged item stays active: without AllowWhenBlockedByActiveItem, no window would be hovered
    if (payload == nullptr || !payload->IsDataType(ASSET_PAYLOAD) || !ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
        ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
        return;
    }
    const std::filesystem::path path(static_cast<const char *>(payload->Data));
    const ImVec2 mouse = ImGui::GetMousePos();
    const Vector2f world = window.mapPixelToWorld(Vector2i{static_cast<int>(mouse.x), static_cast<int>(mouse.y)});

    this->drop(registry, Asset{path, AssetKind::Texture}, world);
}

} // namespace younity::editor

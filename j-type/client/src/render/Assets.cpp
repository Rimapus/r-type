/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Assets
** File description:
** Assets source file
*/

#include "render/Assets.hpp"

#include <expected>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>

#include "jtype/common/Log.hpp"
#include "jtype/common/Paths.hpp"

namespace jtype {

namespace {

// The sprites of the game, relative to the assets folder
const std::filesystem::path SPRITES_FILE = "data/sprites.txt";

/**
 * @brief Read the sprite file for the resource manager, which only keeps the kind of error.
 *
 * @param path The file
 *
 * @return The sprites, or AssetError::LoadFailed after printing why the file is wrong.
 */
std::expected<SpriteSheets, younity::assets::AssetError> loadSprites(const std::filesystem::path &path)
{
    auto sheets = loadSpriteSheets(path);

    if (!sheets) {
        logger().error("{}", sheets.error());
        return std::unexpected(younity::assets::AssetError::LoadFailed);
    }
    return std::move(*sheets);
}

} // namespace

Assets::Assets(void)
    : _resources(executableDir() / "assets")
{
    this->_resources.setLoader<younity::render::Texture>(younity::render::Texture::loadFromFile);
    this->_resources.setLoader<younity::audio::SoundBuffer>(younity::audio::SoundBuffer::loadFromFile);
    this->_resources.setLoader<SpriteSheets>(loadSprites);
    auto sprites = this->_resources.load<SpriteSheets>(SPRITES_FILE);

    if (!sprites) {
        throw std::runtime_error(std::format("cannot load the sprites {}", SPRITES_FILE.string()));
    }
    this->_sprites = *sprites;
}

std::shared_ptr<const younity::render::Texture> Assets::loadTexture(const std::filesystem::path &path)
{
    auto texture = this->_resources.load<younity::render::Texture>(path);

    if (!texture) {
        throw std::runtime_error(std::format("cannot load the texture {}", path.string()));
    }
    return *texture;
}

std::shared_ptr<const younity::audio::SoundBuffer> Assets::loadSound(const std::filesystem::path &path)
{
    auto sound = this->_resources.load<younity::audio::SoundBuffer>(path);

    if (!sound) {
        throw std::runtime_error(std::format("cannot load the sound {}", path.string()));
    }
    return *sound;
}

std::filesystem::path Assets::getPath(const std::filesystem::path &path) const
{
    return this->_resources.getRoot() / path;
}

const SpriteSheet &Assets::getSprite(EntityType type) const
{
    return this->getSprite(getSpriteName(type));
}

const SpriteSheet &Assets::getSprite(std::string_view name) const
{
    const auto found = this->_sprites->find(std::string(name));

    if (found == this->_sprites->end()) {
        throw std::runtime_error(std::format("no sprite \"{}\" in {}", name, SPRITES_FILE.string()));
    }
    return found->second;
}

younity::assets::ResourceManager &Assets::getResources(void) noexcept
{
    return this->_resources;
}

} // namespace jtype

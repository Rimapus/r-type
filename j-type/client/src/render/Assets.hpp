/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Assets
** File description:
** Assets header file
*/

#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

#include "jtype/common/EntityType.hpp"
#include "jtype/common/SpriteSheets.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/audio/SoundBuffer.hpp"
#include "younity/render/Texture.hpp"

namespace jtype {

/**
 * @brief The textures, the sprites and the sounds of the assets folder next to the executable, each file loaded once.
 */
class Assets
{
public:
    /**
     * @brief Find the assets folder and read the sprites of assets/data/sprites.txt.
     *
     * @throw std::runtime_error if the folder of the executable cannot be found or the sprite file cannot be read.
     */
    Assets(void);

    /**
     * @brief Load a texture, the program stops if it is missing.
     *
     * @param path The file, relative to the assets folder
     *
     * @return The texture.
     *
     * @throw std::runtime_error if the texture cannot be loaded.
     */
    [[nodiscard]] std::shared_ptr<const younity::render::Texture> loadTexture(const std::filesystem::path &path);

    /**
     * @brief Load a sound, the program stops if it is missing.
     *
     * @param path The file, relative to the assets folder
     *
     * @return The sound.
     *
     * @throw std::runtime_error if the sound cannot be loaded.
     */
    [[nodiscard]] std::shared_ptr<const younity::audio::SoundBuffer> loadSound(const std::filesystem::path &path);

    /**
     * @brief Find a file of the assets folder, for what is read from its file instead of loaded once, like the music.
     *
     * @param path The file, relative to the assets folder
     *
     * @return The path of the file.
     */
    [[nodiscard]] std::filesystem::path getPath(const std::filesystem::path &path) const;

    /**
     * @brief Find how an entity of a type is drawn.
     *
     * @param type The type of the entity
     *
     * @return Its sprite.
     *
     * @throw std::runtime_error if the sprite file has no sprite for this type.
     */
    [[nodiscard]] const SpriteSheet &getSprite(EntityType type) const;

    /**
     * @brief Find a sprite by its name in the sprite file.
     *
     * @param name The name of the sprite
     *
     * @return The sprite.
     *
     * @throw std::runtime_error if the sprite file has no sprite of this name.
     */
    [[nodiscard]] const SpriteSheet &getSprite(std::string_view name) const;

    /**
     * @brief Get the resource manager of the assets folder, e.g. for the asset browser of the editor of the engine.
     *
     * @return The resource manager, with the loaders of the textures and of the sounds.
     */
    [[nodiscard]] younity::assets::ResourceManager &getResources(void) noexcept;

private:
    younity::assets::ResourceManager _resources;
    std::shared_ptr<const SpriteSheets> _sprites;
};

} // namespace jtype

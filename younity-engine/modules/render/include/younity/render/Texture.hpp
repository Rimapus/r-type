/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/Texture
** File description:
** Texture header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <memory>

#include "younity/assets/AssetError.hpp"
#include "younity/core/Vector2.hpp"

namespace sf {
class Texture;
}

namespace younity::render {

/**
 * @brief An image loaded in the graphics card, drawn by the sprites.
 *        Load it through the resource manager, so a file used by many sprites is read once:
 *
 *        resources.setLoader<render::Texture>(render::Texture::loadFromFile);
 *        auto ship = resources.load<render::Texture>("sprites/ship.png");
 */
class Texture
{
public:
    /**
     * @brief The image in the graphics library, defined inside the render module only:
     *        SFML never appears in this header.
     */
    struct Impl;

    /**
     * @brief Read an image file, e.g. a PNG. Its signature is the one of an assets::Loader<Texture>.
     *
     * @param path The file of the image
     *
     * @return The texture, AssetError::NotFound if there is no file at this path,
     *         or AssetError::LoadFailed if the file is not an image the graphics library can read.
     */
    [[nodiscard]] static std::expected<Texture, assets::AssetError> loadFromFile(const std::filesystem::path &path);

    ~Texture(void);

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    Texture(Texture &&) noexcept;
    Texture &operator=(Texture &&) noexcept;

    /**
     * @brief Get the size of the image.
     *
     * @return The width and the height, in pixels, 0 by 0 for a texture moved from.
     */
    [[nodiscard]] Vector2<unsigned int> getSize(void) const;

    /**
     * @brief Get the image in the graphics library, for the render module that draws it.
     *        Outside of it, Impl is an incomplete type: nothing can be done with it.
     *
     * @return The image, nullptr for a texture moved from.
     */
    [[nodiscard]] const Impl *getImpl(void) const noexcept;

private:
    explicit Texture(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> _impl;
};

} // namespace younity::render

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/Font
** File description:
** Font header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <memory>

#include "younity/assets/AssetError.hpp"

namespace younity::render {

/**
 * @brief A font, e.g. a TTF file, used by the texts.
 *        Load it through the resource manager, so a font used by many texts is read once:
 *
 *        resources.setLoader<render::Font>(render::Font::loadFromFile);
 *        auto font = resources.load<render::Font>("fonts/arcade.ttf");
 */
class Font
{
public:
    /**
     * @brief The font in the graphics library, defined inside the render module only:
     *        SFML never appears in this header.
     */
    struct Impl;

    /**
     * @brief Open a font file. Its signature is the one of an assets::Loader<Font>.
     *        The whole file is read when it opens, so it can move or disappear afterwards.
     *
     * @param path The file of the font
     *
     * @return The font, AssetError::NotFound if there is no file at this path,
     *         or AssetError::LoadFailed if the file is not a font the graphics library can read.
     */
    [[nodiscard]] static std::expected<Font, assets::AssetError> loadFromFile(const std::filesystem::path &path);

    ~Font(void);

    Font(const Font &) = delete;
    Font &operator=(const Font &) = delete;
    Font(Font &&) noexcept;
    Font &operator=(Font &&) noexcept;

    /**
     * @brief Get the font in the graphics library, for the render module that draws it.
     *        Outside of it, Impl is an incomplete type: nothing can be done with it.
     *
     * @return The font, nullptr for a font moved from.
     */
    [[nodiscard]] const Impl *getImpl(void) const noexcept;

private:
    explicit Font(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> _impl;
};

} // namespace younity::render

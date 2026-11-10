/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/Font
** File description:
** Font source file
*/

#include "younity/render/Font.hpp"
#include "FontImpl.hpp"
#include <system_error>
#include <utility>

namespace younity::render {

std::expected<Font, assets::AssetError> Font::loadFromFile(const std::filesystem::path &path)
{
    std::error_code error;

    // Checked first: SFML gives the same failure for a missing file and for a broken one
    if (!std::filesystem::is_regular_file(path, error)) {
        return std::unexpected(assets::AssetError::NotFound);
    }

    auto impl = std::make_unique<Impl>();

    if (!impl->font.openFromFile(path)) {
        return std::unexpected(assets::AssetError::LoadFailed);
    }
    return Font(std::move(impl));
}

Font::Font(std::unique_ptr<Impl> impl)
    : _impl(std::move(impl)) {}

Font::~Font(void) = default;

Font::Font(Font &&) noexcept = default;

Font &Font::operator=(Font &&) noexcept = default;

const Font::Impl *Font::getImpl(void) const noexcept
{
    return this->_impl.get();
}

} // namespace younity::render

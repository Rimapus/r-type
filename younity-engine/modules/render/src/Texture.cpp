/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/Texture
** File description:
** Texture source file
*/

#include "younity/render/Texture.hpp"
#include "TextureImpl.hpp"
#include <system_error>
#include <utility>

namespace younity::render {

std::expected<Texture, assets::AssetError> Texture::loadFromFile(const std::filesystem::path &path)
{
    std::error_code error;

    // Checked first: SFML gives the same failure for a missing file and for a broken one
    if (!std::filesystem::is_regular_file(path, error)) {
        return std::unexpected(assets::AssetError::NotFound);
    }

    auto impl = std::make_unique<Impl>();

    if (!impl->texture.loadFromFile(path)) {
        return std::unexpected(assets::AssetError::LoadFailed);
    }
    return Texture(std::move(impl));
}

Texture::Texture(std::unique_ptr<Impl> impl)
    : _impl(std::move(impl)) {}

Texture::~Texture(void) = default;

Texture::Texture(Texture &&) noexcept = default;

Texture &Texture::operator=(Texture &&) noexcept = default;

Vector2<unsigned int> Texture::getSize(void) const
{
    // A moved-from texture has no image left
    if (this->_impl == nullptr) {
        return {0, 0};
    }

    const sf::Vector2u size = this->_impl->texture.getSize();

    return {size.x, size.y};
}

const Texture::Impl *Texture::getImpl(void) const noexcept
{
    return this->_impl.get();
}

} // namespace younity::render

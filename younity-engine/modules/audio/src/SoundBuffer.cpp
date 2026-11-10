/*
** EPITECH PROJECT, 2026
** younity-engine/modules/audio/src/SoundBuffer
** File description:
** SoundBuffer source file
*/

#include "younity/audio/SoundBuffer.hpp"

#include "SoundBufferImpl.hpp"

#include <system_error>
#include <utility>

namespace younity::audio {

std::expected<SoundBuffer, assets::AssetError> SoundBuffer::loadFromFile(const std::filesystem::path &path)
{
    std::error_code error;

    // Checked first: SFML gives the same failure for a missing file and for a broken one
    if (!std::filesystem::is_regular_file(path, error)) {
        return std::unexpected(assets::AssetError::NotFound);
    }
    auto impl = std::make_unique<Impl>();

    if (!impl->buffer.loadFromFile(path)) {
        return std::unexpected(assets::AssetError::LoadFailed);
    }
    return SoundBuffer(std::move(impl));
}

SoundBuffer::SoundBuffer(std::unique_ptr<Impl> impl)
    : _impl(std::move(impl)) {}

SoundBuffer::~SoundBuffer(void) = default;

SoundBuffer::SoundBuffer(SoundBuffer &&) noexcept = default;

SoundBuffer &SoundBuffer::operator=(SoundBuffer &&) noexcept = default;

float SoundBuffer::getDuration(void) const
{
    // A moved-from sound has no samples left
    if (this->_impl == nullptr) {
        return 0.0F;
    }
    return this->_impl->buffer.getDuration().asSeconds();
}

const SoundBuffer::Impl *SoundBuffer::getImpl(void) const noexcept
{
    return this->_impl.get();
}

} // namespace younity::audio

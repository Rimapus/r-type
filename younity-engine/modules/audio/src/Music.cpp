/*
** EPITECH PROJECT, 2026
** younity-engine/modules/audio/src/Music
** File description:
** Music source file
*/

#include "younity/audio/Music.hpp"

#include "Volume.hpp"

#include <SFML/Audio/Music.hpp>
#include <system_error>
#include <utility>

namespace younity::audio {

struct Music::Impl
{
    sf::Music music;
    // Kept here, not read back from SFML: without audio output, SFML forgets them
    float volume = 1.0F;
    bool looping = false;
};

std::expected<Music, assets::AssetError> Music::openFromFile(const std::filesystem::path &path)
{
    std::error_code error;

    // Checked first: SFML gives the same failure for a missing file and for a broken one
    if (!std::filesystem::is_regular_file(path, error)) {
        return std::unexpected(assets::AssetError::NotFound);
    }
    auto impl = std::make_unique<Impl>();

    if (!impl->music.openFromFile(path)) {
        return std::unexpected(assets::AssetError::LoadFailed);
    }
    return Music(std::move(impl));
}

Music::Music(std::unique_ptr<Impl> impl)
    : _impl(std::move(impl)) {}

Music::~Music(void) = default;

Music::Music(Music &&) noexcept = default;

Music &Music::operator=(Music &&) noexcept = default;

void Music::play(void)
{
    if (this->_impl != nullptr) {
        this->_impl->music.play();
    }
}

void Music::pause(void)
{
    if (this->_impl != nullptr) {
        this->_impl->music.pause();
    }
}

void Music::stop(void)
{
    if (this->_impl != nullptr) {
        this->_impl->music.stop();
    }
}

bool Music::isPlaying(void) const
{
    return this->_impl != nullptr && this->_impl->music.getStatus() == sf::SoundSource::Status::Playing;
}

void Music::setLooping(bool looping)
{
    if (this->_impl != nullptr) {
        this->_impl->looping = looping;
        this->_impl->music.setLooping(looping);
    }
}

bool Music::isLooping(void) const
{
    return this->_impl != nullptr && this->_impl->looping;
}

void Music::setVolume(float volume)
{
    if (this->_impl != nullptr) {
        this->_impl->volume = clampVolume(volume);
        this->_impl->music.setVolume(this->_impl->volume * SFML_FULL_VOLUME);
    }
}

float Music::getVolume(void) const
{
    if (this->_impl == nullptr) {
        return 0.0F;
    }
    return this->_impl->volume;
}

} // namespace younity::audio

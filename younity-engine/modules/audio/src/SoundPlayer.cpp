/*
** EPITECH PROJECT, 2026
** younity-engine/modules/audio/src/SoundPlayer
** File description:
** SoundPlayer source file
*/

#include "younity/audio/SoundPlayer.hpp"

#include "SoundBufferImpl.hpp"
#include "Volume.hpp"

#include <SFML/Audio/Sound.hpp>
#include <algorithm>
#include <cstdint>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace younity::audio {

namespace {

/**
 * @brief One sound playing at a time, and the buffer it reads.
 */
struct Voice
{
    // Declared before the sound, so destroyed after it: an SFML sound must never outlive its buffer
    std::shared_ptr<const SoundBuffer> buffer;
    // Empty until the first sound: an SFML sound cannot exist without a buffer
    std::optional<sf::Sound> sound;
    // The volume asked for this sound, before the one of the player
    float volume = 1.0F;
    // The number of the play that started this sound, to find the oldest
    std::uint64_t startedAt = 0;
};

bool isPlaying(const Voice &voice)
{
    return voice.sound && voice.sound->getStatus() == sf::SoundSource::Status::Playing;
}

/**
 * @brief Find the voice for a new sound: a silent one, or the one of the oldest sound.
 *
 * @param voices The voices of the player, at least one
 *
 * @return The voice.
 */
Voice &pickVoice(std::vector<Voice> &voices)
{
    const auto silent = std::ranges::find_if_not(voices, isPlaying);

    if (silent != voices.end()) {
        return *silent;
    }
    return *std::ranges::min_element(voices, {}, &Voice::startedAt);
}

} // namespace

struct SoundPlayer::Impl
{
    std::vector<Voice> voices;
    float volume = 1.0F;
    std::uint64_t plays = 0;
};

SoundPlayer::SoundPlayer(std::size_t voices)
    : _impl(std::make_unique<Impl>())
{
    if (voices == 0) {
        throw std::invalid_argument("younity::audio::SoundPlayer: a player needs at least one voice");
    }
    this->_impl->voices.resize(voices);
}

SoundPlayer::~SoundPlayer(void) = default;

bool SoundPlayer::play(std::shared_ptr<const SoundBuffer> buffer, float volume)
{
    if (buffer == nullptr || buffer->getImpl() == nullptr) {
        return false;
    }
    const sf::SoundBuffer &samples = buffer->getImpl()->buffer;
    Voice &voice = pickVoice(this->_impl->voices);
    // Owned by the voice before the sound reads it, even if SFML throws; the previous one is freed
    // at the end of the function, once the sound no longer reads it
    const std::shared_ptr<const SoundBuffer> previous = std::exchange(voice.buffer, std::move(buffer));

    try {
        if (voice.sound) {
            voice.sound->stop();
            voice.sound->setBuffer(samples);
        } else {
            voice.sound.emplace(samples);
        }
    } catch (...) {
        // Destroyed while both buffers are alive: the sound may still read either of them
        voice.sound.reset();
        throw;
    }
    voice.volume = clampVolume(volume);
    voice.startedAt = ++this->_impl->plays;
    voice.sound->setVolume(voice.volume * this->_impl->volume * SFML_FULL_VOLUME);
    voice.sound->play();
    return true;
}

void SoundPlayer::stopAll(void)
{
    for (Voice &voice : this->_impl->voices) {
        if (voice.sound) {
            voice.sound->stop();
        }
    }
}

void SoundPlayer::setVolume(float volume)
{
    this->_impl->volume = clampVolume(volume);
    for (Voice &voice : this->_impl->voices) {
        if (voice.sound) {
            voice.sound->setVolume(voice.volume * this->_impl->volume * SFML_FULL_VOLUME);
        }
    }
}

float SoundPlayer::getVolume(void) const noexcept
{
    return this->_impl->volume;
}

std::size_t SoundPlayer::getVoiceCount(void) const noexcept
{
    return this->_impl->voices.size();
}

std::size_t SoundPlayer::getPlayingCount(void) const
{
    return static_cast<std::size_t>(std::ranges::count_if(this->_impl->voices, isPlaying));
}

} // namespace younity::audio

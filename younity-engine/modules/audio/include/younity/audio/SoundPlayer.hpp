/*
** EPITECH PROJECT, 2026
** younity-engine/modules/audio/include/younity/audio/SoundPlayer
** File description:
** SoundPlayer header file
*/

#pragma once

#include <cstddef>
#include <memory>

#include "younity/audio/SoundBuffer.hpp"

namespace younity::audio {

/**
 * @brief Number of voices of a player by default: enough for the shots, hits and explosions of a busy screen.
 */
inline constexpr std::size_t DEFAULT_VOICES = 32;

/**
 * @brief Plays the short sounds of the game, without the game keeping them:
 *
 *        audio::SoundPlayer sounds;
 *        sounds.play(*shot, 0.8F);
 *
 *        The player has a fixed number of voices, one sound each at a time. When they are all
 *        playing, a new sound replaces the one that started first.
 *        Use it from one thread only, the one of the game loop.
 */
class SoundPlayer
{
public:
    /**
     * @brief Create a player that plays up to a number of sounds at the same time.
     *
     * @param voices The number of sounds playing at the same time
     *
     * @throw std::invalid_argument if voices is 0.
     */
    explicit SoundPlayer(std::size_t voices = DEFAULT_VOICES);

    ~SoundPlayer(void);
    SoundPlayer(const SoundPlayer &) = delete;
    SoundPlayer &operator=(const SoundPlayer &) = delete;
    SoundPlayer(SoundPlayer &&) = delete;
    SoundPlayer &operator=(SoundPlayer &&) = delete;

    /**
     * @brief Play a sound once, on a free voice, or on the voice of the oldest sound if none is free.
     *        The voice keeps the buffer until it plays another sound, so it may be unloaded meanwhile.
     *
     * @param buffer The sound to play
     * @param volume Its volume, from 0 for silence to 1 for full volume, scaled by the volume of the player
     *
     * @return false if there is no sound to play: an empty pointer or a sound moved from.
     */
    bool play(std::shared_ptr<const SoundBuffer> buffer, float volume = 1.0F);

    /**
     * @brief Stop every sound playing.
     */
    void stopAll(void);

    /**
     * @brief Set the volume of all the sounds, the ones playing included.
     *
     * @param volume From 0 for silence to 1 for full volume, kept in that range
     */
    void setVolume(float volume);

    /**
     * @brief Get the volume of all the sounds.
     *
     * @return The volume, from 0 to 1.
     */
    [[nodiscard]] float getVolume(void) const noexcept;

    /**
     * @brief Get the number of sounds that can play at the same time.
     *
     * @return The number of voices.
     */
    [[nodiscard]] std::size_t getVoiceCount(void) const noexcept;

    /**
     * @brief Get the number of sounds playing now. It stays 0 on a machine without audio output.
     *
     * @return The number of busy voices.
     */
    [[nodiscard]] std::size_t getPlayingCount(void) const;

private:
    /**
     * @brief The voices in the audio library, defined in the source file only.
     */
    struct Impl;

    std::unique_ptr<Impl> _impl;
};

} // namespace younity::audio

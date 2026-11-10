/*
** EPITECH PROJECT, 2026
** younity-engine/modules/audio/include/younity/audio/SoundBuffer
** File description:
** SoundBuffer header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <memory>

#include "younity/assets/AssetError.hpp"

namespace younity::audio {

/**
 * @brief A short sound read into memory, like a shot or an explosion, played by a SoundPlayer.
 *        Load it through the resource manager, so a sound played a hundred times is read once:
 *
 *        resources.setLoader<audio::SoundBuffer>(audio::SoundBuffer::loadFromFile);
 *        auto shot = resources.load<audio::SoundBuffer>("sounds/shot.wav");
 */
class SoundBuffer
{
public:
    /**
     * @brief The samples in the audio library, defined inside the audio module only:
     *        SFML never appears in this header.
     */
    struct Impl;

    /**
     * @brief Read a sound file, e.g. a WAV, an OGG or a FLAC. Its signature is the one of an assets::Loader<SoundBuffer>.
     *
     * @param path The file of the sound
     *
     * @return The sound, AssetError::NotFound if there is no file at this path,
     *         or AssetError::LoadFailed if the file is not a sound the audio library can read.
     */
    [[nodiscard]] static std::expected<SoundBuffer, assets::AssetError> loadFromFile(const std::filesystem::path &path);

    ~SoundBuffer(void);
    SoundBuffer(const SoundBuffer &) = delete;
    SoundBuffer &operator=(const SoundBuffer &) = delete;
    SoundBuffer(SoundBuffer &&) noexcept;
    SoundBuffer &operator=(SoundBuffer &&) noexcept;

    /**
     * @brief Get how long the sound lasts.
     *
     * @return The duration, in seconds, 0 for a sound moved from.
     */
    [[nodiscard]] float getDuration(void) const;

    /**
     * @brief Get the samples in the audio library, for the audio module that plays them.
     *        Outside of it, Impl is an incomplete type: nothing can be done with it.
     *
     * @return The samples, nullptr for a sound moved from.
     */
    [[nodiscard]] const Impl *getImpl(void) const noexcept;

private:
    explicit SoundBuffer(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> _impl;
};

} // namespace younity::audio

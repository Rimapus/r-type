/*
** EPITECH PROJECT, 2026
** younity-engine/modules/audio/include/younity/audio/Music
** File description:
** Music header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <memory>

#include "younity/assets/AssetError.hpp"

namespace younity::audio {

/**
 * @brief A long sound, like the music of a level, read from its file while it plays instead of all at once.
 *        The game owns it directly, not through the resource manager: a music is changed when it plays,
 *        and the manager only shares resources that cannot be changed. Use it from one thread only.
 *
 *        auto music = audio::Music::openFromFile("assets/music/level1.ogg");
 *        music->setLooping(true);
 *        music->play();
 */
class Music
{
public:
    /**
     * @brief Open a sound file, e.g. an OGG, a WAV or a FLAC. The file must stay in place while the music exists.
     *
     * @param path The file of the music
     *
     * @return The music, stopped, AssetError::NotFound if there is no file at this path,
     *         or AssetError::LoadFailed if the file is not a sound the audio library can read.
     */
    [[nodiscard]] static std::expected<Music, assets::AssetError> openFromFile(const std::filesystem::path &path);

    ~Music(void);
    Music(const Music &) = delete;
    Music &operator=(const Music &) = delete;
    Music(Music &&) noexcept;
    Music &operator=(Music &&) noexcept;

    /**
     * @brief Start the music, from the beginning or from where it was paused. Nothing for a music moved from.
     */
    void play(void);

    /**
     * @brief Pause the music, play starts it again from there. Nothing for a music moved from.
     */
    void pause(void);

    /**
     * @brief Stop the music, play starts it again from the beginning. Nothing for a music moved from.
     */
    void stop(void);

    /**
     * @brief Check if the music is playing. It never is on a machine without audio output.
     *
     * @return true if it is playing, false if it is paused, stopped, over, or moved from.
     */
    [[nodiscard]] bool isPlaying(void) const;

    /**
     * @brief Set if the music starts again from the beginning when it is over. Nothing for a music moved from.
     *
     * @param looping true to play it again and again
     */
    void setLooping(bool looping);

    /**
     * @brief Check if the music starts again when it is over.
     *
     * @return true if it loops, false by default and for a music moved from.
     */
    [[nodiscard]] bool isLooping(void) const;

    /**
     * @brief Set the volume of the music. Nothing for a music moved from.
     *
     * @param volume From 0 for silence to 1 for full volume, kept in that range
     */
    void setVolume(float volume);

    /**
     * @brief Get the volume of the music.
     *
     * @return The volume, from 0 to 1, 1 by default and 0 for a music moved from.
     */
    [[nodiscard]] float getVolume(void) const;

private:
    /**
     * @brief The music in the audio library, defined in the source file only.
     */
    struct Impl;

    explicit Music(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> _impl;
};

} // namespace younity::audio

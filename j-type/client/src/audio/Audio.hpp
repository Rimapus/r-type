/*
** EPITECH PROJECT, 2026
** j-type/client/src/audio/Audio
** File description:
** Audio header file
*/

#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

#include "audio/SoundCues.hpp"
#include "render/Assets.hpp"
#include "settings/Settings.hpp"
#include "younity/audio/Music.hpp"
#include "younity/audio/SoundBuffer.hpp"
#include "younity/audio/SoundPlayer.hpp"

namespace jtype {

/**
 * @brief The sound of the game: the music, which plays while a game runs, and the sounds.
 *        On a machine without audio output, nothing is heard and nothing fails.
 */
class Audio
{
public:
    /**
     * @brief Load the sounds and start the music.
     *
     * @param assets The sounds and the music, they must outlive the audio
     *
     * @throw std::runtime_error if a sound or the music cannot be loaded.
     */
    explicit Audio(Assets &assets);

    /**
     * @brief Play sounds, each at its own volume.
     *
     * @param sounds The sounds to play
     */
    void play(const std::vector<Sound> &sounds);

    /**
     * @brief Stop the music when the game is over, and play it again from the beginning when the next game starts.
     *
     * @param over true while the game is over, as in the last scoreboard
     */
    void setGameOver(bool over);

    /**
     * @brief Set the volume of the music and of the sounds, kept while the sound is muted.
     *
     * @param settings The volumes chosen by the player
     */
    void setVolumes(const Settings &settings);

    /**
     * @brief Silence the music and the sounds, or bring them back.
     */
    void toggleMute(void);

private:
    /**
     * @brief Give the music and the sounds their volume, or silence them while the sound is muted.
     */
    void applyVolumes(void);

    // One buffer per value of Sound
    static constexpr std::size_t SOUND_COUNT = 6;

    std::array<std::shared_ptr<const younity::audio::SoundBuffer>, SOUND_COUNT> _sounds;
    younity::audio::SoundPlayer _player;
    younity::audio::Music _music;
    // The volumes chosen by the player, from 0 to 1
    float _musicVolume = 1.0F;
    float _effectsVolume = 1.0F;
    bool _muted = false;
    bool _gameOver = false;
};

} // namespace jtype

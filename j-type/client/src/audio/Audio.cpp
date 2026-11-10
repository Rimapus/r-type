/*
** EPITECH PROJECT, 2026
** j-type/client/src/audio/Audio
** File description:
** Audio source file
*/

#include "audio/Audio.hpp"

#include <array>
#include <expected>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <utility>

namespace jtype {

namespace {

/**
 * @brief A sound file and the volume it plays at.
 */
struct SoundFile
{
    const char *path;
    float volume;
};

// In the order of Sound. The shots are the most frequent sounds, so they are the quietest
constexpr std::array<SoundFile, 6> SOUND_FILES{
    {
     {"audio/sounds/player_shot.wav", 0.35F},
     {"audio/sounds/enemy_shot.wav", 0.3F},
     {"audio/sounds/explosion.wav", 0.6F},
     {"audio/sounds/player_hit.wav", 0.8F},
     {"audio/sounds/player_out.wav", 0.9F},
     {"audio/sounds/game_over.wav", 1.0F},
     }
};

// Under the sounds, so the shots and the explosions are still heard over it
constexpr float MUSIC_VOLUME = 0.4F;

const std::filesystem::path MUSIC_FILE = "audio/music/space_battle.ogg";

/**
 * @brief Open the music of the game.
 *
 * @param assets The assets folder
 *
 * @return The music, stopped.
 *
 * @throw std::runtime_error if the music cannot be opened.
 */
younity::audio::Music openMusic(const Assets &assets)
{
    auto music = younity::audio::Music::openFromFile(assets.getPath(MUSIC_FILE));

    if (!music) {
        throw std::runtime_error(std::format("cannot open the music {}", MUSIC_FILE.string()));
    }
    return std::move(*music);
}

} // namespace

Audio::Audio(Assets &assets)
    : _music(openMusic(assets))
{
    static_assert(SOUND_FILES.size() == SOUND_COUNT);
    for (std::size_t sound = 0; sound < SOUND_COUNT; ++sound) {
        this->_sounds[sound] = assets.loadSound(SOUND_FILES[sound].path);
    }
    this->_music.setLooping(true);
    this->applyVolumes();
    this->_music.play();
}

void Audio::play(const std::vector<Sound> &sounds)
{
    for (const Sound sound : sounds) {
        const auto index = static_cast<std::size_t>(sound);

        (void)this->_player.play(this->_sounds[index], SOUND_FILES[index].volume);
    }
}

void Audio::setGameOver(bool over)
{
    if (over == this->_gameOver) {
        return;
    }
    this->_gameOver = over;
    // Stopped instead of paused, so the next game starts its music from the beginning
    if (over) {
        this->_music.stop();
    } else {
        this->_music.play();
    }
}

void Audio::setVolumes(const Settings &settings)
{
    this->_musicVolume = static_cast<float>(settings.musicVolume) / static_cast<float>(MAX_VOLUME);
    this->_effectsVolume = static_cast<float>(settings.effectsVolume) / static_cast<float>(MAX_VOLUME);
    this->applyVolumes();
}

void Audio::toggleMute(void)
{
    this->_muted = !this->_muted;
    this->applyVolumes();
}

void Audio::applyVolumes(void)
{
    this->_player.setVolume(this->_muted ? 0.0F : this->_effectsVolume);
    this->_music.setVolume(this->_muted ? 0.0F : MUSIC_VOLUME * this->_musicVolume);
}

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/audio/Music
** File description:
** Music unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <limits>
#include <utility>

#include "younity/audio/Music.hpp"

using younity::assets::AssetError;
using younity::audio::Music;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "audio" / "fixtures";

Music openBeep(void)
{
    auto music = Music::openFromFile(FIXTURES / "tone_2s.wav");

    cr_assert(music.has_value());
    return std::move(*music);
}

} // namespace

Test(audio_music, missing_file_is_not_found)
{
    const auto music = Music::openFromFile("does/not/exist.ogg");

    cr_assert_not(music.has_value());
    cr_assert_eq(music.error(), AssetError::NotFound);
}

Test(audio_music, folder_is_not_found)
{
    const auto music = Music::openFromFile(FIXTURES);

    cr_assert_not(music.has_value());
    cr_assert_eq(music.error(), AssetError::NotFound);
}

Test(audio_music, file_that_is_not_a_sound_fails_to_open)
{
    const auto music = Music::openFromFile(FIXTURES / "not_a_sound.wav");

    cr_assert_not(music.has_value());
    cr_assert_eq(music.error(), AssetError::LoadFailed);
}

Test(audio_music, does_not_play_once_opened)
{
    const Music music = openBeep();

    cr_assert_not(music.isPlaying());
}

Test(audio_music, plays_until_stopped)
{
    Music music = openBeep();

    music.setLooping(true);
    music.play();
    if (!music.isPlaying()) {
        cr_skip_test("no audio output to play the music");
    }
    music.stop();
    cr_assert_not(music.isPlaying());
}

Test(audio_music, pause_stops_the_playing)
{
    Music music = openBeep();

    music.setLooping(true);
    music.play();
    if (!music.isPlaying()) {
        cr_skip_test("no audio output to play the music");
    }
    music.pause();
    cr_assert_not(music.isPlaying());
}

Test(audio_music, does_not_loop_at_first)
{
    const Music music = openBeep();

    cr_assert_not(music.isLooping());
}

Test(audio_music, looping_can_be_turned_on)
{
    Music music = openBeep();

    music.setLooping(true);
    cr_assert(music.isLooping());
}

Test(audio_music, volume_is_full_at_first)
{
    const Music music = openBeep();

    cr_assert_float_eq(music.getVolume(), 1.0F, 0.0001F);
}

Test(audio_music, volume_is_kept_between_zero_and_one)
{
    Music music = openBeep();

    music.setVolume(0.25F);
    cr_assert_float_eq(music.getVolume(), 0.25F, 0.0001F);
    music.setVolume(3.0F);
    cr_assert_float_eq(music.getVolume(), 1.0F, 0.0001F);
    music.setVolume(-3.0F);
    cr_assert_float_eq(music.getVolume(), 0.0F, 0.0001F);
}

Test(audio_music, a_nan_volume_is_silence)
{
    Music music = openBeep();

    music.setVolume(std::numeric_limits<float>::quiet_NaN());
    cr_assert_float_eq(music.getVolume(), 0.0F, 0.0001F);
}

Test(audio_music, moved_from_music_does_nothing)
{
    Music music = openBeep();
    const Music moved(std::move(music));

    // NOLINTBEGIN(bugprone-use-after-move): the moved-from state is what is tested
    music.play();
    music.setLooping(true);
    music.setVolume(0.5F);
    cr_assert_not(music.isPlaying());
    cr_assert_not(music.isLooping());
    cr_assert_float_eq(music.getVolume(), 0.0F, 0.0001F);
    // NOLINTEND(bugprone-use-after-move)
}

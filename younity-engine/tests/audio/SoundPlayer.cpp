/*
** EPITECH PROJECT, 2026
** younity-engine/tests/audio/SoundPlayer
** File description:
** SoundPlayer unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <limits>
#include <memory>
#include <stdexcept>

#include "younity/audio/SoundBuffer.hpp"
#include "younity/audio/SoundPlayer.hpp"

using younity::audio::SoundBuffer;
using younity::audio::SoundPlayer;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "audio" / "fixtures";

// Long enough to still play when the test checks it, even on a busy machine
std::shared_ptr<const SoundBuffer> loadBeep(void)
{
    auto buffer = SoundBuffer::loadFromFile(FIXTURES / "tone_2s.wav");

    cr_assert(buffer.has_value());
    return std::make_shared<const SoundBuffer>(std::move(*buffer));
}

// A sound only plays with an audio output: a machine without one (e.g. a CI container) never starts it
void skipIfNothingPlays(const SoundPlayer &player)
{
    if (player.getPlayingCount() == 0) {
        cr_skip_test("no audio output to play the sounds");
    }
}

} // namespace

Test(audio_sound_player, has_the_default_voice_count)
{
    const SoundPlayer player;

    cr_assert_eq(player.getVoiceCount(), younity::audio::DEFAULT_VOICES);
}

Test(audio_sound_player, has_the_given_voice_count)
{
    const SoundPlayer player(4);

    cr_assert_eq(player.getVoiceCount(), 4U);
}

Test(audio_sound_player, zero_voice_is_refused)
{
    cr_assert_throw(SoundPlayer(0), std::invalid_argument);
}

Test(audio_sound_player, nothing_plays_at_first)
{
    const SoundPlayer player;

    cr_assert_eq(player.getPlayingCount(), 0U);
}

Test(audio_sound_player, plays_a_sound)
{
    SoundPlayer player;

    cr_assert(player.play(loadBeep()));
}

Test(audio_sound_player, does_not_play_without_a_buffer)
{
    SoundPlayer player;

    cr_assert_not(player.play(nullptr));
    cr_assert_eq(player.getPlayingCount(), 0U);
}

Test(audio_sound_player, keeps_the_buffer_while_it_may_play)
{
    SoundPlayer player;
    std::shared_ptr<const SoundBuffer> buffer = loadBeep();
    const std::weak_ptr<const SoundBuffer> watched = buffer;

    cr_assert(player.play(std::move(buffer)));
    cr_assert_not(watched.expired());
}

Test(audio_sound_player, full_player_replaces_the_oldest_sound)
{
    SoundPlayer player(2);
    const std::shared_ptr<const SoundBuffer> beep = loadBeep();

    cr_assert(player.play(beep));
    skipIfNothingPlays(player);
    cr_assert(player.play(beep));
    cr_assert(player.play(beep));
    cr_assert_eq(player.getPlayingCount(), 2U);
}

Test(audio_sound_player, stop_all_stops_every_sound)
{
    SoundPlayer player;
    const std::shared_ptr<const SoundBuffer> beep = loadBeep();

    cr_assert(player.play(beep));
    cr_assert(player.play(beep));
    skipIfNothingPlays(player);
    player.stopAll();
    cr_assert_eq(player.getPlayingCount(), 0U);
}

Test(audio_sound_player, volume_is_full_at_first)
{
    const SoundPlayer player;

    cr_assert_float_eq(player.getVolume(), 1.0F, 0.0001F);
}

Test(audio_sound_player, volume_is_kept_between_zero_and_one)
{
    SoundPlayer player;

    player.setVolume(0.5F);
    cr_assert_float_eq(player.getVolume(), 0.5F, 0.0001F);
    player.setVolume(2.0F);
    cr_assert_float_eq(player.getVolume(), 1.0F, 0.0001F);
    player.setVolume(-1.0F);
    cr_assert_float_eq(player.getVolume(), 0.0F, 0.0001F);
}

Test(audio_sound_player, a_nan_volume_is_silence)
{
    SoundPlayer player;

    player.setVolume(std::numeric_limits<float>::quiet_NaN());
    cr_assert_float_eq(player.getVolume(), 0.0F, 0.0001F);
}

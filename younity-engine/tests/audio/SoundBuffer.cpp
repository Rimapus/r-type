/*
** EPITECH PROJECT, 2026
** younity-engine/tests/audio/SoundBuffer
** File description:
** SoundBuffer unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <utility>

#include "younity/assets/ResourceManager.hpp"
#include "younity/audio/SoundBuffer.hpp"

using younity::assets::AssetError;
using younity::audio::SoundBuffer;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "audio" / "fixtures";

} // namespace

Test(audio_sound_buffer, missing_file_is_not_found)
{
    const auto buffer = SoundBuffer::loadFromFile("does/not/exist.wav");

    cr_assert_not(buffer.has_value());
    cr_assert_eq(buffer.error(), AssetError::NotFound);
}

Test(audio_sound_buffer, folder_is_not_found)
{
    const auto buffer = SoundBuffer::loadFromFile(FIXTURES);

    cr_assert_not(buffer.has_value());
    cr_assert_eq(buffer.error(), AssetError::NotFound);
}

Test(audio_sound_buffer, file_that_is_not_a_sound_fails_to_load)
{
    const auto buffer = SoundBuffer::loadFromFile(FIXTURES / "not_a_sound.wav");

    cr_assert_not(buffer.has_value());
    cr_assert_eq(buffer.error(), AssetError::LoadFailed);
}

Test(audio_sound_buffer, loads_a_sound)
{
    const auto buffer = SoundBuffer::loadFromFile(FIXTURES / "beep_100ms.wav");

    cr_assert(buffer.has_value());
    cr_assert_not_null(buffer->getImpl());
}

Test(audio_sound_buffer, duration_is_the_one_of_the_file)
{
    const auto buffer = SoundBuffer::loadFromFile(FIXTURES / "beep_100ms.wav");

    cr_assert(buffer.has_value());
    cr_assert_float_eq(buffer->getDuration(), 0.1F, 0.001F);
}

Test(audio_sound_buffer, moved_from_buffer_is_empty)
{
    auto loaded = SoundBuffer::loadFromFile(FIXTURES / "beep_100ms.wav");

    cr_assert(loaded.has_value());
    const SoundBuffer moved(std::move(*loaded));
    cr_assert_not_null(moved.getImpl());
    // NOLINTNEXTLINE(bugprone-use-after-move): the moved-from state is what is tested
    cr_assert_null(loaded->getImpl());
    cr_assert_float_eq(loaded->getDuration(), 0.0F, 0.0001F);
}

Test(audio_sound_buffer, resource_manager_loads_and_shares_it)
{
    younity::assets::ResourceManager resources(FIXTURES);

    resources.setLoader<SoundBuffer>(SoundBuffer::loadFromFile);
    const auto first = resources.load<SoundBuffer>("beep_100ms.wav");
    const auto second = resources.load<SoundBuffer>("beep_100ms.wav");
    cr_assert(first.has_value() && second.has_value());
    cr_assert_eq(first->get(), second->get());
}

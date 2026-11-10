/*
** EPITECH PROJECT, 2026
** younity-engine/modules/audio/src/SoundBufferImpl
** File description:
** SoundBufferImpl header file, private to the audio module
*/

#pragma once

#include "younity/audio/SoundBuffer.hpp"

#include <SFML/Audio/SoundBuffer.hpp>

namespace younity::audio {

/**
 * @brief The SFML sound buffer behind a SoundBuffer. In src/: the users of the module never see it.
 */
struct SoundBuffer::Impl
{
    sf::SoundBuffer buffer;
};

} // namespace younity::audio

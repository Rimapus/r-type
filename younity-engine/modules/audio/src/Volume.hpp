/*
** EPITECH PROJECT, 2026
** younity-engine/modules/audio/src/Volume
** File description:
** Volume header file, private to the audio module
*/

#pragma once

#include <algorithm>
#include <cmath>

namespace younity::audio {

// The engine takes a volume from 0 to 1, SFML one from 0 to 100
inline constexpr float SFML_FULL_VOLUME = 100.0F;

/**
 * @brief Keep a volume between silence and full volume.
 *
 * @param volume The volume asked for, 1 being full volume. A NaN is silence.
 *
 * @return The volume, from 0 to 1.
 */
[[nodiscard]] inline float clampVolume(float volume) noexcept
{
    if (std::isnan(volume)) {
        return 0.0F;
    }
    return std::clamp(volume, 0.0F, 1.0F);
}

} // namespace younity::audio

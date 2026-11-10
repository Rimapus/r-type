/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Captions
** File description:
** Captions header file
*/

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include "audio/SoundCues.hpp"
#include "render/HudLayout.hpp"

namespace jtype {

// The time a caption stays after the last time its sound was heard, in seconds
inline constexpr float CAPTION_DURATION = 2.0F;

// The end of that time, during which the caption fades out, in seconds
inline constexpr float CAPTION_FADE = 0.5F;

/**
 * @brief The captions of the sounds, for the players who cannot hear them: a line at the bottom of the screen for
 *        each sound that tells something, kept CAPTION_DURATION after the last time it was heard, and faded out during
 *        the last CAPTION_FADE of it.
 */
class Captions
{
public:
    /**
     * @brief Show the line of each sound, or keep it longer if it is already shown.
     *
     * @param sounds The sounds just played
     */
    void hear(const std::vector<Sound> &sounds);

    /**
     * @brief Count the time down, fade the lines out, and hide the lines not heard for CAPTION_DURATION.
     *
     * @param step The duration of the step, in seconds
     *
     * @return true if a line appeared, faded or went since the last call, so the lines are drawn again.
     */
    [[nodiscard]] bool update(float step);

    /**
     * @brief Lay out the lines shown, centered one under the other at the bottom of the screen, in the order of Sound.
     *        A line fading out is more see-through as its time runs out.
     *
     * @return The texts to draw.
     */
    [[nodiscard]] std::vector<HudText> layout(void) const;

private:
    // The time left to the line of each value of Sound, 0 while it is hidden
    std::array<float, static_cast<std::size_t>(Sound::GameOver) + 1> _left{};
    bool _changed = false;
};

/**
 * @brief Find the caption of a sound.
 *
 * @param sound The sound
 *
 * @return The caption, or nothing for a sound whose event the screen already shows: the shot of a player, the game
 *         over.
 */
[[nodiscard]] std::optional<std::string_view> getCaption(Sound sound);

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/components/Animation
** File description:
** Animation header file
*/

#pragma once

#include <cstddef>
#include <optional>
#include <utility>

#include "younity/core/Rect.hpp"
#include "younity/render/AnimationClip.hpp"

namespace younity::render {

/**
 * @brief The animation an entity plays, moved forward by the AnimationSystem.
 *        Its current frame is the part of the texture of the Sprite that is drawn, in place of its textureRect.
 */
struct Animation
{
    /**
     * @brief Play another clip from its first frame, e.g. an explosion after the flight of a ship.
     *        It restarts even if it is the same clip: call it when the animation changes, not at every step.
     *
     * @param next The clip to play
     */
    void play(AnimationClip next)
    {
        this->clip = std::move(next);
        this->frame = 0;
        this->elapsed = 0.0F;
        this->finished = false;
    }

    /**
     * @brief Get the part of the texture to show now.
     *
     * @return The current frame, or nothing if the clip has no frame there.
     */
    [[nodiscard]] std::optional<Recti> getCurrentFrame(void) const
    {
        if (this->frame >= this->clip.frames.size()) {
            return std::nullopt;
        }
        return this->clip.frames[this->frame];
    }

    AnimationClip clip;
    // The number of the frame shown, in the frames of the clip. Out of them, the Sprite keeps its textureRect
    // until the next step, which goes back to the first frame, or to the last one if the clip does not loop
    std::size_t frame = 0;
    // The time the current frame has been shown, in seconds
    float elapsed = 0.0F;
    // true once a clip that does not loop has shown its last frame for its whole duration
    bool finished = false;
};

} // namespace younity::render

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/AnimationClip
** File description:
** AnimationClip header file
*/

#pragma once

#include <vector>

#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"

namespace younity::render {

/**
 * @brief Where the frames of an animation are in a sheet of frames of the same size,
 *        numbered from 0, left to right then top to bottom.
 */
struct GridLayout
{
    // The size of one frame, in pixels
    Vector2i frameSize{};
    // The number of frames on a row of the sheet
    int columns = 1;
    // The number of the first frame of the animation
    int first = 0;
    // The number of frames of the animation
    int count = 1;
};

/**
 * @brief An animation of a spritesheet: the parts of the texture shown one after the other.
 *        Given to an Animation component, it is shared by value: a few rectangles per entity.
 */
struct AnimationClip
{
    /**
     * @brief Read the frames of an animation in a sheet laid out as a grid, e.g. the 4 frames of the first row:
     *        AnimationClip::fromGrid({.frameSize = {32, 32}, .columns = 4, .first = 0, .count = 4}, 0.1F);
     *
     * @param grid Where the frames are in the sheet
     * @param frameDuration How long each frame is shown, in seconds
     * @param loop true to start again after the last frame, false to stop on it
     *
     * @return The clip.
     *
     * @throw std::invalid_argument if the frame size, the columns or the count are not positive,
     *        if the first frame is negative, if the duration is not a positive number,
     *        or if a frame would be placed beyond the int coordinates of a texture.
     */
    [[nodiscard]] static AnimationClip fromGrid(const GridLayout &grid, float frameDuration, bool loop = true);

    // The parts of the texture to show, in pixels, in order. Without any, the animation does nothing and never ends
    std::vector<Recti> frames;
    // How long each frame is shown, in seconds. 0 or less freezes the animation on its frame
    float frameDuration = 0.1F;
    // true to start again after the last frame, false to stop on it and publish an AnimationFinishedEvent
    bool loop = true;
};

} // namespace younity::render

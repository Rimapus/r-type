/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/AnimationClip
** File description:
** AnimationClip source file
*/

#include "younity/render/AnimationClip.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace younity::render {

AnimationClip AnimationClip::fromGrid(const GridLayout &grid, float frameDuration, bool loop)
{
    if (grid.frameSize.x <= 0 || grid.frameSize.y <= 0 || grid.columns <= 0 || grid.first < 0 || grid.count <= 0) {
        throw std::invalid_argument("younity::render::AnimationClip: the grid needs a positive frame size, columns and count, and a first frame from 0");
    }
    if (!std::isfinite(frameDuration) || frameDuration <= 0.0F) {
        throw std::invalid_argument("younity::render::AnimationClip: the frame duration must be a positive number");
    }

    // In 64 bits: the far corner of every frame must fit in an int, the coordinates of a texture
    constexpr std::int64_t MAX_INT = std::numeric_limits<int>::max();
    const std::int64_t last = static_cast<std::int64_t>(grid.first) + grid.count - 1;
    const std::int64_t lastColumn = std::min<std::int64_t>(last, grid.columns - 1);
    const std::int64_t lastRow = last / grid.columns;

    if (last > MAX_INT || (lastColumn + 1) * grid.frameSize.x > MAX_INT || (lastRow + 1) * grid.frameSize.y > MAX_INT) {
        throw std::invalid_argument("younity::render::AnimationClip: the grid goes beyond the coordinates of a texture");
    }

    AnimationClip clip;

    clip.frameDuration = frameDuration;
    clip.loop = loop;
    clip.frames.reserve(static_cast<std::size_t>(grid.count));
    for (int number = grid.first; number < grid.first + grid.count; ++number) {
        const int column = number % grid.columns;
        const int row = number / grid.columns;

        clip.frames.push_back(Recti{column * grid.frameSize.x, row * grid.frameSize.y, grid.frameSize.x, grid.frameSize.y});
    }
    return clip;
}

} // namespace younity::render

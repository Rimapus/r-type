/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/PixelPerfect
** File description:
** PixelPerfect source file
*/

#include "younity/render/PixelPerfect.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace younity::render {

Rectf getPixelPerfectArea(Vector2<unsigned int> logicalSize, Vector2<unsigned int> windowSize)
{
    if (logicalSize.x == 0 || logicalSize.y == 0 || windowSize.x == 0 || windowSize.y == 0) {
        throw std::invalid_argument("younity::render::getPixelPerfectArea: a size is 0");
    }
    const float fit = std::min(static_cast<float>(windowSize.x) / static_cast<float>(logicalSize.x),
                               static_cast<float>(windowSize.y) / static_cast<float>(logicalSize.y));
    // Below 1, no whole factor fits: shrinking blurs the pixels, but shows the whole game
    const float scale = fit >= 1.0F ? std::floor(fit) : fit;
    const float width = static_cast<float>(logicalSize.x) * scale;
    const float height = static_cast<float>(logicalSize.y) * scale;

    // Rounded, so the game starts on a whole screen pixel
    return Rectf{std::floor((static_cast<float>(windowSize.x) - width) / 2.0F),
                 std::floor((static_cast<float>(windowSize.y) - height) / 2.0F), width, height};
}

Vector2f mapPixelToLogical(Vector2f pixel, const Rectf &area, Vector2<unsigned int> logicalSize)
{
    return Vector2f{(pixel.x - area.left) * static_cast<float>(logicalSize.x) / area.width,
                    (pixel.y - area.top) * static_cast<float>(logicalSize.y) / area.height};
}

Vector2f mapLogicalToPixel(Vector2f logical, const Rectf &area, Vector2<unsigned int> logicalSize)
{
    return Vector2f{area.left + (logical.x * area.width / static_cast<float>(logicalSize.x)),
                    area.top + (logical.y * area.height / static_cast<float>(logicalSize.y))};
}

} // namespace younity::render

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/components/Sprite
** File description:
** Sprite header file
*/

#pragma once

#include <memory>
#include <optional>

#include "younity/core/Color.hpp"
#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/render/Texture.hpp"

namespace younity::render {

/**
 * @brief The image of an entity, drawn at its Transform by drawEntities().
 */
struct Sprite
{
    // Shared with the resource manager: the texture lives as long as a sprite uses it. Nothing is drawn without one.
    std::shared_ptr<const Texture> texture;
    // The part of the texture to draw, in pixels, e.g. one image of a sheet. The whole texture without one.
    std::optional<Recti> textureRect;
    // Lower layers are drawn first, so higher ones are on top of them, e.g. background -1, ships 0, interface 10
    int layer = 0;
    // The point of the image placed at the position of the Transform, and the center of its rotation, in pixels
    Vector2f origin{};
    // false hides the sprite without removing it, e.g. to blink
    bool visible = true;
    // Multiplies the colors of the texture: opaque white by default, so the image is drawn as it is. A lower alpha fades
    // it, e.g. Color{255, 255, 255, 128} draws it half see-through
    Color color{};
};

} // namespace younity::render

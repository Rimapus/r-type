/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/Renderer
** File description:
** Renderer header file
*/

#pragma once

#include <array>
#include <optional>
#include <vector>

#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"

namespace younity::render {

class Window;

/**
 * @brief What is drawn for an entity: its Sprite or its Text.
 */
enum class DrawKind {
    Sprite,
    Text,
};

/**
 * @brief One thing to draw: the Sprite or the Text of an entity.
 */
struct DrawItem
{
    ecs::Entity entity;
    DrawKind kind;

    bool operator==(const DrawItem &other) const = default;
};

/**
 * @brief Get what to draw, in the order it is drawn: the visible Sprites and Texts of the entities having a Transform,
 *        sorted by layer, then by entity id in the same layer, the Sprite of an entity before its Text.
 *
 * @param registry The registry owning the entities
 *
 * @return The items, the first one drawn first.
 */
[[nodiscard]] std::vector<DrawItem> getDrawOrder(const ecs::Registry &registry);

/**
 * @brief Get the part of the texture drawn for an entity: the current frame of its Animation if it has one,
 *        the textureRect of its Sprite otherwise.
 *
 * @param registry The registry owning the entity
 * @param entity An entity having a Sprite
 *
 * @return The part of the texture, or nothing to draw the whole texture.
 *
 * @throw std::out_of_range if the entity has no Sprite.
 */
[[nodiscard]] std::optional<Recti> getTextureRect(const ecs::Registry &registry, ecs::Entity entity);

/**
 * @brief Get the corners of the rectangle drawn for an item, placed, rotated and scaled by the Transform of its entity,
 *        e.g. to outline the entity. A Sprite without texture and a Text without font or characters cover no area.
 *        A Text is measured with getTextBounds(), so it needs a graphics context.
 *
 * @param registry The registry owning the entity
 * @param item An item of getDrawOrder()
 *
 * @return The top left, top right, bottom right and bottom left corners before the rotation, in the world, in pixels.
 *
 * @throw std::out_of_range if the entity has no Transform, or not the Sprite or Text of the item.
 */
[[nodiscard]] std::array<Vector2f, 4> getDrawCorners(const ecs::Registry &registry, const DrawItem &item);

/**
 * @brief Get the entity drawn at a point, e.g. under the mouse cursor: the one drawn last, on top of the others,
 *        among the items of getDrawOrder() whose rectangle contains the point. A Text is measured with
 *        getTextBounds(), so it needs a graphics context.
 *
 * @param registry The registry owning the entities
 * @param point The point in the world, in pixels
 *
 * @return The entity, or nothing if no Sprite or Text is drawn there.
 */
[[nodiscard]] std::optional<ecs::Entity> getEntityAt(const ecs::Registry &registry, Vector2f point);

/**
 * @brief Draw the sprites and the texts of the registry in the window, layer by layer.
 *        Called once per frame between window.clear() and window.display(),
 *        outside of the fixed steps: it is not a system of the registry.
 *
 * @param registry The registry owning the entities
 * @param window The window to draw in
 */
void drawEntities(const ecs::Registry &registry, Window &window);

} // namespace younity::render

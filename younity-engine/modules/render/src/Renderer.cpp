/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/Renderer
** File description:
** Renderer source file
*/

#include "younity/render/Renderer.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/render/Window.hpp"
#include "younity/render/components/Animation.hpp"
#include "younity/render/components/Sprite.hpp"
#include "younity/render/components/Text.hpp"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <numbers>
#include <ranges>

namespace younity::render {

namespace {

/**
 * @brief Get the visible drawables of a type, as draw items sorted by entity id.
 *
 * @tparam Drawable Sprite or Text
 * @param registry The registry owning the entities
 * @param kind The kind of the items
 *
 * @return The items of the entities having a Transform and a visible Drawable.
 */
template <typename Drawable>
std::vector<DrawItem> getVisible(const ecs::Registry &registry, DrawKind kind)
{
    std::vector<DrawItem> items;

    for (const ecs::Entity entity : registry.getEntitiesWith<ecs::Transform, Drawable>()) {
        if (registry.getComponent<Drawable>(entity).visible) {
            items.push_back(DrawItem{entity, kind});
        }
    }
    return items;
}

/**
 * @brief Get the layer of a draw item.
 *
 * @param registry The registry owning the entity
 * @param item The item
 *
 * @return The layer of its Sprite or of its Text.
 */
int getLayer(const ecs::Registry &registry, const DrawItem &item)
{
    if (item.kind == DrawKind::Sprite) {
        return registry.getComponent<Sprite>(item.entity).layer;
    }
    return registry.getComponent<Text>(item.entity).layer;
}

/**
 * @brief Get the rectangle drawn for an item, before its origin and the Transform of its entity are applied.
 *
 * @param registry The registry owning the entity
 * @param item The item
 *
 * @return The rectangle of its Sprite or of its Text, empty when nothing is drawn.
 */
Rectf getLocalBounds(const ecs::Registry &registry, const DrawItem &item)
{
    if (item.kind == DrawKind::Text) {
        return getTextBounds(registry.getComponent<Text>(item.entity));
    }

    const Sprite &sprite = registry.getComponent<Sprite>(item.entity);

    if (sprite.texture == nullptr || sprite.texture->getImpl() == nullptr) {
        return Rectf{};
    }

    const std::optional<Recti> rect = getTextureRect(registry, item.entity);

    if (rect) {
        // A negative size flips the image: it covers the same area
        return Rectf{0.0F, 0.0F, static_cast<float>(std::abs(rect->width)), static_cast<float>(std::abs(rect->height))};
    }

    const Vector2<unsigned int> size = sprite.texture->getSize();

    return Rectf{0.0F, 0.0F, static_cast<float>(size.x), static_cast<float>(size.y)};
}

/**
 * @brief Get the origin of an item: the point placed at the position of the Transform.
 *
 * @param registry The registry owning the entity
 * @param item The item
 *
 * @return The origin of its Sprite or of its Text, in pixels.
 */
Vector2f getOrigin(const ecs::Registry &registry, const DrawItem &item)
{
    if (item.kind == DrawKind::Sprite) {
        return registry.getComponent<Sprite>(item.entity).origin;
    }
    return registry.getComponent<Text>(item.entity).origin;
}

/**
 * @brief Place a point of an item in the world: scale it, rotate it, then move it to the position, as SFML draws it.
 *
 * @param transform The Transform of the entity
 * @param local The point, from the origin of the item
 *
 * @return The point in the world.
 */
Vector2f toWorld(const ecs::Transform &transform, Vector2f local)
{
    const float angle = transform.rotation * std::numbers::pi_v<float> / 180.0F;
    const float x = local.x * transform.scale.x;
    const float y = local.y * transform.scale.y;

    return Vector2f{transform.position.x + (x * std::cos(angle)) - (y * std::sin(angle)),
                    transform.position.y + (x * std::sin(angle)) + (y * std::cos(angle))};
}

/**
 * @brief Bring a point of the world back to an item: the reverse of toWorld().
 *
 * @param transform The Transform of the entity
 * @param world The point in the world
 *
 * @return The point, from the origin of the item, or nothing if a scale of 0 flattens the item.
 */
std::optional<Vector2f> toLocal(const ecs::Transform &transform, Vector2f world)
{
    if (transform.scale.x == 0.0F || transform.scale.y == 0.0F) {
        return std::nullopt;
    }

    const float angle = transform.rotation * std::numbers::pi_v<float> / 180.0F;
    const Vector2f offset = world - transform.position;

    return Vector2f{((offset.x * std::cos(angle)) + (offset.y * std::sin(angle))) / transform.scale.x,
                    ((offset.y * std::cos(angle)) - (offset.x * std::sin(angle))) / transform.scale.y};
}

} // namespace

std::vector<DrawItem> getDrawOrder(const ecs::Registry &registry)
{
    const std::vector<DrawItem> sprites = getVisible<Sprite>(registry, DrawKind::Sprite);
    const std::vector<DrawItem> texts = getVisible<Text>(registry, DrawKind::Text);
    std::vector<DrawItem> order;

    // By entity id, the Sprite of an entity before its Text: the stable sort keeps this order inside a layer
    std::ranges::merge(sprites, texts, std::back_inserter(order), [](const DrawItem &left, const DrawItem &right) {
        return left.entity.getId() < right.entity.getId();
    });
    std::ranges::stable_sort(order, {}, [&registry](const DrawItem &item) {
        return getLayer(registry, item);
    });
    return order;
}

std::optional<Recti> getTextureRect(const ecs::Registry &registry, ecs::Entity entity)
{
    if (registry.hasComponent<Animation>(entity)) {
        const std::optional<Recti> frame = registry.getComponent<Animation>(entity).getCurrentFrame();

        if (frame) {
            return frame;
        }
    }
    return registry.getComponent<Sprite>(entity).textureRect;
}

std::array<Vector2f, 4> getDrawCorners(const ecs::Registry &registry, const DrawItem &item)
{
    const ecs::Transform &transform = registry.getComponent<ecs::Transform>(item.entity);
    const Rectf bounds = getLocalBounds(registry, item);
    const Vector2f origin = getOrigin(registry, item);
    const float left = bounds.left - origin.x;
    const float top = bounds.top - origin.y;
    const float right = left + bounds.width;
    const float bottom = top + bounds.height;

    return {toWorld(transform, Vector2f{left, top}), toWorld(transform, Vector2f{right, top}),
            toWorld(transform, Vector2f{right, bottom}), toWorld(transform, Vector2f{left, bottom})};
}

std::optional<ecs::Entity> getEntityAt(const ecs::Registry &registry, Vector2f point)
{
    const std::vector<DrawItem> order = getDrawOrder(registry);

    // The last item drawn covers the ones before it
    for (const DrawItem &item : std::views::reverse(order)) {
        const std::optional<Vector2f> local = toLocal(registry.getComponent<ecs::Transform>(item.entity), point);

        if (local && getLocalBounds(registry, item).contains(*local + getOrigin(registry, item))) {
            return item.entity;
        }
    }
    return std::nullopt;
}

void drawEntities(const ecs::Registry &registry, Window &window)
{
    for (const DrawItem &item : getDrawOrder(registry)) {
        const ecs::Transform &transform = registry.getComponent<ecs::Transform>(item.entity);

        if (item.kind == DrawKind::Text) {
            window.draw(registry.getComponent<Text>(item.entity), transform);
            continue;
        }

        Sprite shown = registry.getComponent<Sprite>(item.entity);

        shown.textureRect = getTextureRect(registry, item.entity);
        window.draw(shown, transform);
    }
}

} // namespace younity::render

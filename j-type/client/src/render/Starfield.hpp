/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Starfield
** File description:
** Starfield header file
*/

#pragma once

#include <memory>
#include <vector>

#include "younity/core/Color.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/render/Texture.hpp"

namespace jtype {

// The starfield with the high contrast setting: a third of its brightness, so the ships, the Bydos and the shots stand out
inline constexpr younity::Color DIMMED_STARFIELD{85, 85, 85};

// The sprite layer of the farthest image of the space background, far behind the game entities
inline constexpr int BACKGROUND_LAYER = -100;

/**
 * @brief The space background: layers of images scrolling to the left, the farthest ones slower.
 */
class Starfield
{
public:
    /**
     * @brief Create an empty background.
     *
     * @param registry The registry holding the images, it must outlive the background
     * @param firstLayer The sprite layer of the first image, the next ones one step closer: far behind the game entities
     *                   by default, e.g. TERRAIN_LAYER for the ceiling and the floor
     */
    explicit Starfield(younity::ecs::Registry &registry, int firstLayer = BACKGROUND_LAYER);

    /**
     * @brief Add a layer in front of the ones already added.
     *
     * @param texture The image of the layer, its right edge must follow its left edge
     * @param speed The scroll speed of the layer, in pixels of the game area per second
     * @param top The top of the image, in pixels of the game area: 0 for an image as high as the game area
     */
    void addLayer(const std::shared_ptr<const younity::render::Texture> &texture, float speed, float top = 0.0F);

    /**
     * @brief Move every layer to the left, by its speed.
     *
     * @param step The duration of the step, in seconds
     */
    void scroll(float step);

    /**
     * @brief Multiply the colors of every layer, the ones added later included, e.g. DIMMED_STARFIELD to darken them.
     *
     * @param color The color, opaque white to draw the layers as they are
     */
    void setColor(younity::Color color);

private:
    /**
     * @brief The copies of one image, placed one after the other so they always cover the game area.
     */
    struct Layer
    {
        std::vector<younity::ecs::Entity> copies;
        float width;
        float speed;
        float top;
        float offset = 0.0F;
    };

    /**
     * @brief Place the copies of a layer at its offset.
     *
     * @param layer The layer to place
     */
    void place(const Layer &layer);

    younity::ecs::Registry &_registry;
    int _firstLayer;
    std::vector<Layer> _layers;
    younity::Color _color{};
};

} // namespace jtype

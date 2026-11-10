/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Overlay
** File description:
** Overlay header file
*/

#pragma once

#include <memory>
#include <vector>

#include "render/Assets.hpp"
#include "render/HudLayout.hpp"
#include "younity/core/Color.hpp"
#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/render/Texture.hpp"

namespace jtype {

/**
 * @brief Sprites drawn over every entity of the game, like the texts of the HUD and of the menus, removed together.
 */
class Overlay
{
public:
    /**
     * @brief Create an empty overlay.
     *
     * @param registry The registry holding the sprites, it must outlive the overlay
     * @param assets The textures of the font, they must outlive the overlay
     */
    Overlay(younity::ecs::Registry &registry, Assets &assets);

    /**
     * @brief Create the sprites of the characters of a text.
     *
     * @param text The text
     */
    void write(const HudText &text);

    /**
     * @brief Create a sprite.
     *
     * @param texture The image
     * @param frame The part of the image to draw
     * @param position The top left corner, in pixels of the game area
     * @param size The times it is drawn bigger than the game area
     * @param tint Multiplied with the image, white by default to draw it as it is
     */
    void addSprite(const std::shared_ptr<const younity::render::Texture> &texture, younity::Recti frame,
                   younity::Vector2f position, float size, younity::Color tint = {});

    /**
     * @brief Remove every sprite created.
     */
    void clear(void);

private:
    younity::ecs::Registry &_registry;
    Assets &_assets;
    std::vector<younity::ecs::Entity> _sprites;
};

} // namespace jtype

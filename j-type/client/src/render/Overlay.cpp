/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Overlay
** File description:
** Overlay source file
*/

#include "render/Overlay.hpp"

#include "render/BitmapFont.hpp"
#include "render/Screen.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/render/components/Sprite.hpp"

namespace jtype {

namespace {

// Over every entity of the game
constexpr int OVERLAY_LAYER = 10;

} // namespace

Overlay::Overlay(younity::ecs::Registry &registry, Assets &assets)
    : _registry(registry),
      _assets(assets)
{
}

void Overlay::write(const HudText &text)
{
    const std::shared_ptr<const younity::render::Texture> font = this->_assets.loadTexture(getFontPath(text.color));

    for (const Glyph &glyph : layoutText(text.string)) {
        this->addSprite(font, glyph.frame, {text.position.x + glyph.x * text.size, text.position.y}, text.size,
                        text.tint);
    }
}

void Overlay::addSprite(const std::shared_ptr<const younity::render::Texture> &texture, younity::Recti frame,
                        younity::Vector2f position, float size, younity::Color tint)
{
    const younity::ecs::Entity sprite = this->_registry.createEntity();
    younity::ecs::Transform transform = toScreen(position.x, position.y);
    younity::render::Sprite image{texture, frame, OVERLAY_LAYER};

    transform.scale *= size;
    image.color = tint;
    this->_registry.addComponent(sprite, image);
    this->_registry.addComponent(sprite, transform);
    this->_sprites.push_back(sprite);
}

void Overlay::clear(void)
{
    for (const younity::ecs::Entity sprite : this->_sprites) {
        this->_registry.destroyEntity(sprite);
    }
    this->_sprites.clear();
}

} // namespace jtype

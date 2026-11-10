/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Starfield
** File description:
** Starfield source file
*/

#include "render/Starfield.hpp"

#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <utility>

#include "jtype/common/Constants.hpp"
#include "render/Screen.hpp"
#include "younity/render/components/Sprite.hpp"

namespace jtype {

Starfield::Starfield(younity::ecs::Registry &registry, int firstLayer)
    : _registry(registry),
      _firstLayer(firstLayer)
{
}

void Starfield::addLayer(const std::shared_ptr<const younity::render::Texture> &texture, float speed, float top)
{
    const auto width = static_cast<float>(texture->getSize().x);

    if (width <= 0.0F) {
        throw std::invalid_argument("a starfield layer cannot be an empty image");
    }
    // Enough copies to cover the game area at any offset: the last one comes in while the first one goes out
    const auto count = static_cast<std::size_t>(std::ceil(WORLD_WIDTH / width)) + 1;
    const int layer = this->_firstLayer + static_cast<int>(this->_layers.size());
    Layer added{{}, width, speed, top};

    for (std::size_t i = 0; i < count; ++i) {
        const younity::ecs::Entity copy = this->_registry.createEntity();
        younity::render::Sprite image{texture, std::nullopt, layer};

        image.color = this->_color;
        this->_registry.addComponent(copy, image);
        added.copies.push_back(copy);
    }
    this->place(added);
    this->_layers.push_back(std::move(added));
}

void Starfield::scroll(float step)
{
    for (Layer &layer : this->_layers) {
        layer.offset = std::fmod(layer.offset + layer.speed * step, layer.width);
        this->place(layer);
    }
}

void Starfield::setColor(younity::Color color)
{
    this->_color = color;
    for (const Layer &layer : this->_layers) {
        for (const younity::ecs::Entity &copy : layer.copies) {
            this->_registry.getComponent<younity::render::Sprite>(copy).color = color;
        }
    }
}

void Starfield::place(const Layer &layer)
{
    float x = -layer.offset;

    for (const younity::ecs::Entity &copy : layer.copies) {
        this->_registry.addComponent(copy, toScreen(x, layer.top));
        x += layer.width;
    }
}

} // namespace jtype

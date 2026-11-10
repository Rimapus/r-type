/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/Text
** File description:
** Text source file
*/

#include "younity/render/components/Text.hpp"
#include "TextConversion.hpp"

namespace younity::render {

Rectf getTextBounds(const Text &text)
{
    const Font::Impl *font = text.font != nullptr ? text.font->getImpl() : nullptr;

    if (font == nullptr || text.string.empty()) {
        return Rectf{0.0F, 0.0F, 0.0F, 0.0F};
    }

    const sf::FloatRect bounds = toSfmlText(text, *font).getLocalBounds();

    return Rectf{bounds.position.x, bounds.position.y, bounds.size.x, bounds.size.y};
}

} // namespace younity::render

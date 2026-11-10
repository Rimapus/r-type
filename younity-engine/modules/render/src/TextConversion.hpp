/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/TextConversion
** File description:
** TextConversion header file, private to the render module
*/

#pragma once

#include "FontImpl.hpp"
#include "younity/render/components/Text.hpp"
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>

namespace younity::render {

/**
 * @brief Build the SFML text of a Text, without its Transform. The font must not be null.
 *
 * @param text The text to convert
 * @param font The font of the text, kept by the SFML text: it must outlive it
 *
 * @return The SFML text, with its string read as UTF-8.
 */
[[nodiscard]] inline sf::Text toSfmlText(const Text &text, const Font::Impl &font)
{
    sf::Text converted(font.font, sf::String::fromUtf8(text.string.begin(), text.string.end()), text.characterSize);

    converted.setFillColor(sf::Color(text.color.r, text.color.g, text.color.b, text.color.a));
    converted.setOrigin({text.origin.x, text.origin.y});
    return converted;
}

} // namespace younity::render

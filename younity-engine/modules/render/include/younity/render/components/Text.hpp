/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/components/Text
** File description:
** Text header file
*/

#pragma once

#include <memory>
#include <string>

#include "younity/core/Color.hpp"
#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/render/Font.hpp"

namespace younity::render {

/**
 * @brief A text shown by an entity, e.g. a score or a menu entry, drawn at its Transform by drawEntities().
 */
struct Text
{
    // Shared with the resource manager: the font lives as long as a text uses it. Nothing is drawn without one.
    std::shared_ptr<const Font> font;
    // In UTF-8, so accents are shown: "Partie terminée". Invalid UTF-8 is not refused, it shows wrong characters
    std::string string;
    // The height of the characters, in pixels
    unsigned int characterSize = 30;
    Color color{};
    // Shared with the sprites: lower layers are drawn first, a text is drawn after a sprite of the same entity and layer
    int layer = 0;
    // The point of the text placed at the position of the Transform, and the center of its rotation, in pixels
    Vector2f origin{};
    // false hides the text without removing it
    bool visible = true;
};

/**
 * @brief Measure a text, e.g. to center it: origin = {bounds.left + bounds.width / 2, bounds.top + bounds.height / 2}.
 *        The Transform is not applied. It renders the glyphs of the font, so it needs a graphics context: a display, which a headless server lacks.
 *
 * @param text The text to measure
 *
 * @return The rectangle covered by the characters, from the origin of the text, in pixels.
 *         Empty without a font or without characters.
 */
[[nodiscard]] Rectf getTextBounds(const Text &text);

} // namespace younity::render

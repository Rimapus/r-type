/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Popups
** File description:
** Popups source file
*/

#include "scenes/Popups.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>

#include "render/BitmapFont.hpp"

namespace jtype {

std::string_view getPowerUpName(PowerUpType type)
{
    switch (type) {
    case PowerUpType::Shield:
        return "SHIELD";
    case PowerUpType::SpreadShot:
        return "SPREAD";
    case PowerUpType::ExtraLife:
        break;
    }
    return "+1 LIFE";
}

void Popups::add(std::string text, younity::Vector2f position, younity::Color color)
{
    if (this->_popups.size() == MAX_POPUPS) {
        this->_popups.erase(this->_popups.begin());
    }
    this->_popups.push_back(Popup{std::move(text), position, color, 0.0F});
    this->_changed = true;
}

bool Popups::update(float step)
{
    const std::size_t before = this->_popups.size();

    for (Popup &popup : this->_popups) {
        popup.elapsed += step;
    }
    std::erase_if(this->_popups, [](const Popup &popup) {
        return popup.elapsed >= POPUP_DURATION;
    });

    // A text moves at each step, and the last one gone must be erased from the screen too
    const bool changed = this->_changed || !this->_popups.empty() || this->_popups.size() != before;

    this->_changed = false;
    return changed;
}

std::vector<HudText> Popups::layout(void) const
{
    std::vector<HudText> texts;

    for (const Popup &popup : this->_popups) {
        const float progress = popup.elapsed / POPUP_DURATION;
        const float width = static_cast<float>(popup.text.size()) * GLYPH_ADVANCE;
        HudText text{
            popup.text,
            {popup.position.x - width / 2.0F,
                   popup.position.y - static_cast<float>(GLYPH_SIZE) - POPUP_RISE * progress},
            FontColor::White,
            1.0F
        };

        text.tint = popup.color;
        text.tint.a = static_cast<std::uint8_t>(255.0F * (1.0F - progress));
        texts.push_back(std::move(text));
    }
    return texts;
}

} // namespace jtype

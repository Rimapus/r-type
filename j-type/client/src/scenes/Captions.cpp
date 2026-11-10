/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Captions
** File description:
** Captions source file
*/

#include "scenes/Captions.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

#include "jtype/common/Constants.hpp"
#include "render/BitmapFont.hpp"
#include "scenes/Menu.hpp"

namespace jtype {

namespace {

// The space between the last line and the bottom of the game area, in pixels of the game area
constexpr float CAPTION_MARGIN = 4.0F;

// From the top of a line to the top of the next one, in pixels of the game area
constexpr float CAPTION_SPACING = 10.0F;

} // namespace

std::optional<std::string_view> getCaption(Sound sound)
{
    switch (sound) {
    case Sound::EnemyShot:
        return "BYDO SHOT";
    case Sound::Explosion:
        return "EXPLOSION";
    case Sound::PlayerHit:
        return "PLAYER HIT";
    case Sound::PlayerOut:
        return "PLAYER OUT";
    case Sound::PlayerShot:
    case Sound::GameOver:
        break;
    }
    return std::nullopt;
}

void Captions::hear(const std::vector<Sound> &sounds)
{
    for (const Sound sound : sounds) {
        if (!getCaption(sound)) {
            continue;
        }
        float &left = this->_left[static_cast<std::size_t>(sound)];

        // A hidden line appears, a line fading out is opaque again
        if (left < CAPTION_FADE) {
            this->_changed = true;
        }
        left = CAPTION_DURATION;
    }
}

bool Captions::update(float step)
{
    for (float &left : this->_left) {
        if (left <= 0.0F) {
            continue;
        }
        left -= step;
        if (left <= 0.0F) {
            left = 0.0F;
        }
        // A line fading out is drawn again at each step, more see-through
        if (left < CAPTION_FADE) {
            this->_changed = true;
        }
    }

    const bool changed = this->_changed;

    this->_changed = false;
    return changed;
}

std::vector<HudText> Captions::layout(void) const
{
    std::vector<std::size_t> shown;
    std::vector<HudText> texts;

    for (std::size_t sound = 0; sound < this->_left.size(); ++sound) {
        if (this->_left[sound] > 0.0F) {
            shown.push_back(sound);
        }
    }
    for (std::size_t line = 0; line < shown.size(); ++line) {
        const float fromBottom = static_cast<float>(shown.size() - 1 - line) * CAPTION_SPACING;
        const float top = WORLD_HEIGHT - CAPTION_MARGIN - static_cast<float>(GLYPH_SIZE) - fromBottom;
        const float left = this->_left[shown[line]];
        HudText text = centerText(std::string(*getCaption(static_cast<Sound>(shown[line]))), top, FontColor::White, 1.0F);

        text.tint.a = static_cast<std::uint8_t>(255.0F * std::min(1.0F, left / CAPTION_FADE));
        texts.push_back(std::move(text));
    }
    return texts;
}

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Notices
** File description:
** Notices source file
*/

#include "scenes/Notices.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>

#include "scenes/Menu.hpp"

namespace jtype {

namespace {

// The top of the first notice, under the scores and the lives of the HUD
constexpr float NOTICE_TOP = 30.0F;
// The distance between the tops of two notices
constexpr float NOTICE_SPACING = 10.0F;

} // namespace

void Notices::announce(std::string text, younity::Color color)
{
    if (this->_notices.size() == MAX_NOTICES) {
        this->_notices.pop_front();
    }
    this->_notices.push_back(Notice{std::move(text), color, NOTICE_DURATION});
    this->_changed = true;
}

bool Notices::update(float step)
{
    for (Notice &notice : this->_notices) {
        notice.left -= step;
        // A notice fading out is drawn again at each step, more see-through
        if (notice.left < NOTICE_FADE) {
            this->_changed = true;
        }
    }
    std::erase_if(this->_notices, [](const Notice &notice) {
        return notice.left <= 0.0F;
    });

    const bool changed = this->_changed;

    this->_changed = false;
    return changed;
}

std::vector<HudText> Notices::layout(void) const
{
    std::vector<HudText> texts;

    for (std::size_t line = 0; line < this->_notices.size(); ++line) {
        const Notice &notice = this->_notices[line];
        HudText text = centerText(notice.text, NOTICE_TOP + static_cast<float>(line) * NOTICE_SPACING, FontColor::White,
                                  1.0F);

        text.tint = notice.color;
        text.tint.a = static_cast<std::uint8_t>(255.0F * std::min(1.0F, notice.left / NOTICE_FADE));
        texts.push_back(std::move(text));
    }
    return texts;
}

} // namespace jtype

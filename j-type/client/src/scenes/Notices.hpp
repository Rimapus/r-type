/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Notices
** File description:
** Notices header file
*/

#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

#include "render/HudLayout.hpp"
#include "younity/core/Color.hpp"

namespace jtype {

// How long a notice stays, in seconds, and the end of it during which it fades out
inline constexpr float NOTICE_DURATION = 3.0F;
inline constexpr float NOTICE_FADE = 0.5F;
// The notices shown at once
inline constexpr std::size_t MAX_NOTICES = 3;

/**
 * @brief The notices of what happened in the game, e.g. "P2 DISCONNECTED": a line each, centered under the HUD, in
 *        the color of the player it is about. The top of the screen tells what happened, the bottom what is heard
 *        (Captions), so a notice is never taken for a caption.
 */
class Notices
{
public:
    /**
     * @brief Show a notice for NOTICE_DURATION seconds, below the ones already shown. Past MAX_NOTICES lines, the
     *        oldest one goes.
     *
     * @param text The text of the notice
     * @param color The color of the player it is about
     */
    void announce(std::string text, younity::Color color);

    /**
     * @brief Move the notices forward in time, so they fade out and go.
     *
     * @param step The duration of the step, in seconds
     *
     * @return true if the notices to draw changed since the last call.
     */
    [[nodiscard]] bool update(float step);

    /**
     * @brief Get the lines to draw, the oldest at the top.
     *
     * @return The texts, more see-through during the last NOTICE_FADE seconds of each notice.
     */
    [[nodiscard]] std::vector<HudText> layout(void) const;

private:
    // A notice shown, with its color and the time it has left
    struct Notice
    {
        std::string text;
        younity::Color color;
        // The seconds before the notice goes
        float left;
    };

    std::deque<Notice> _notices;
    bool _changed = false;
};

} // namespace jtype

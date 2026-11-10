/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Popups
** File description:
** Popups header file
*/

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "jtype/common/PowerUpType.hpp"
#include "render/HudLayout.hpp"
#include "younity/core/Color.hpp"
#include "younity/core/Vector2.hpp"

namespace jtype {

// How long a text is shown, in seconds, and how high it rises in that time, in pixels of the game area
inline constexpr float POPUP_DURATION = 0.8F;
inline constexpr float POPUP_RISE = 8.0F;
// The texts shown at once, enough for a busy wave
inline constexpr std::size_t MAX_POPUPS = 16;

/**
 * @brief Get the text shown over a ship that took a power-up.
 *
 * @param type The power-up
 *
 * @return "+1 LIFE", "SHIELD" or "SPREAD".
 */
[[nodiscard]] std::string_view getPowerUpName(PowerUpType type);

/**
 * @brief The short texts shown where something happened in the game, e.g. "+100" where a player destroyed a Bydo:
 *        each one rises and fades out in POPUP_DURATION seconds.
 */
class Popups
{
public:
    /**
     * @brief Show a text centered on a place of the game area. Past MAX_POPUPS texts, the oldest one goes.
     *
     * @param text The text, e.g. "+100"
     * @param position Where it happened, in pixels of the game area
     * @param color The color of the player it is about
     */
    void add(std::string text, younity::Vector2f position, younity::Color color);

    /**
     * @brief Move the texts forward in time, so they rise, fade out and go.
     *
     * @param step The duration of the step, in seconds
     *
     * @return true if there are texts to draw again, or if the last one just went.
     */
    [[nodiscard]] bool update(float step);

    /**
     * @brief Get the texts to draw.
     *
     * @return The texts, higher and more see-through as they get older.
     */
    [[nodiscard]] std::vector<HudText> layout(void) const;

private:
    // A text shown at a place of the game, with its color and its age
    struct Popup
    {
        std::string text;
        younity::Vector2f position;
        younity::Color color;
        // The seconds since the text appeared
        float elapsed;
    };

    std::vector<Popup> _popups;
    bool _changed = false;
};

} // namespace jtype

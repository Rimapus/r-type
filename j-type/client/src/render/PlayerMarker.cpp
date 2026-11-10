/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/PlayerMarker
** File description:
** PlayerMarker source file
*/

#include "render/PlayerMarker.hpp"

#include <format>
#include <string_view>

#include "render/BitmapFont.hpp"

namespace jtype {

namespace {

// The middle of the hull of a ship, which is drawn a little left of its position
constexpr float SHIP_CENTER_X = -4.0F;

// The middle of the head of the pilot, on the right of the ship, the same on every frame
constexpr float HEAD_CENTER_X = 4.0F;

// The top and the bottom of the hull of a ship, from its position
constexpr float SHIP_TOP = -16.0F;
constexpr float SHIP_BOTTOM = 16.0F;

// The center of the tag, so its arrow points at the head of the pilot
constexpr float TAG_OFFSET_Y = SHIP_TOP - 7.0F;

// The space between the bottom of the ship and the label
constexpr float LABEL_GAP = 2.0F;

constexpr std::string_view LABEL_TEXT = "YOU";

} // namespace

std::optional<PlayerMarker> layoutPlayerMarker(EntityType type, std::optional<std::uint8_t> local)
{
    const std::optional<std::uint8_t> slot = getPlayerSlot(type);

    if (!slot) {
        return std::nullopt;
    }
    PlayerMarker marker{
        .tag = std::format("player_tag_p{}", *slot + 1),
        .tagOffset = {HEAD_CENTER_X, TAG_OFFSET_Y},
        .label = std::nullopt
    };

    if (local && *local == *slot) {
        const float width = static_cast<float>(LABEL_TEXT.size()) * GLYPH_ADVANCE;

        marker.label = HudText{
            .string = std::string(LABEL_TEXT),
            .position = {SHIP_CENTER_X - width / 2.0F, SHIP_BOTTOM + LABEL_GAP},
            .color = FontColor::Amber,
            .size = 1.0F
        };
    }
    return marker;
}

} // namespace jtype

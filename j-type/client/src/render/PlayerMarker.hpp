/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/PlayerMarker
** File description:
** PlayerMarker header file
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "jtype/common/EntityType.hpp"
#include "render/HudLayout.hpp"
#include "younity/core/Vector2.hpp"

namespace jtype {

/**
 * @brief What tells a ship apart from the others: the tag of its player above it, and YOU under the ship of this
 *        client. The tag shows the number of the player, so the ships are not told apart by their color alone.
 */
struct PlayerMarker
{
    // The name of the sprite of the tag, in the sprite file
    std::string tag;
    // The center of the tag, from the position of the ship, in pixels of the game area
    younity::Vector2f tagOffset;
    // YOU, under the ship of this client only; its position is from the position of the ship
    std::optional<HudText> label;
};

/**
 * @brief Find the marker of a ship.
 *
 * @param type The type of the entity
 * @param local The slot of this client, or nothing while it has not joined
 *
 * @return The marker of a ship, nothing for another type.
 */
[[nodiscard]] std::optional<PlayerMarker> layoutPlayerMarker(EntityType type, std::optional<std::uint8_t> local);

} // namespace jtype

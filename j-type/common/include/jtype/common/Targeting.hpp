/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Targeting
** File description:
** Targeting header file
*/

#pragma once

#include <optional>

#include "younity/core/Vector2.hpp"
#include "younity/ecs/Registry.hpp"

namespace jtype {

/**
 * @brief Find the player closest to a position, e.g. the target of a Bydo.
 *
 * @param registry The registry owning the players
 * @param from The position
 *
 * @return The position of the nearest player, or std::nullopt when no player is in the game.
 */
[[nodiscard]] std::optional<younity::Vector2f> findNearestPlayer(const younity::ecs::Registry &registry, younity::Vector2f from);

} // namespace jtype

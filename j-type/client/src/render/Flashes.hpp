/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Flashes
** File description:
** Flashes header file
*/

#pragma once

#include "jtype/common/EntityType.hpp"
#include "younity/core/Color.hpp"

namespace jtype {

/**
 * @brief How an entity is drawn: the scale of its sprite and the color that multiplies it.
 */
struct FlashLook
{
    float scale = 1.0F;
    younity::Color color{};
};

/**
 * @brief Get how an entity is drawn, with the REDUCE FLASHES setting or without: with it, an explosion is smaller and
 *        half see-through, so its flash is dimmer. Everything else is drawn as it is.
 *
 * @param type The type of the entity
 * @param reduceFlashes true if the player asked for fewer flashes
 *
 * @return The scale and the color of its sprite.
 */
[[nodiscard]] FlashLook getFlashLook(EntityType type, bool reduceFlashes) noexcept;

} // namespace jtype

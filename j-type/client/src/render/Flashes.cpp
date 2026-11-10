/*
** EPITECH PROJECT, 2026
** j-type/client/src/render/Flashes
** File description:
** Flashes source file
*/

#include "render/Flashes.hpp"

namespace jtype {

namespace {

// Two thirds of its size, and half see-through over the starfield
constexpr float REDUCED_SCALE = 0.66F;
constexpr younity::Color REDUCED_COLOR{255, 255, 255, 128};

} // namespace

FlashLook getFlashLook(EntityType type, bool reduceFlashes) noexcept
{
    if (reduceFlashes && type == EntityType::Explosion) {
        return FlashLook{REDUCED_SCALE, REDUCED_COLOR};
    }
    return FlashLook{};
}

} // namespace jtype

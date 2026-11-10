/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/Lifetime
** File description:
** Lifetime header file
*/

#pragma once

namespace jtype {

/**
 * @brief An entity that lives for a short time, e.g. an explosion: the LifetimeSystem destroys it at the end.
 */
struct Lifetime
{
    // Seconds left to live
    float remaining = 0.0F;
};

} // namespace jtype

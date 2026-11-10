/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/PowerUp
** File description:
** PowerUp header file
*/

#pragma once

#include "jtype/common/PowerUpType.hpp"

namespace jtype {

/**
 * @brief Marks a power-up left by a Bydo: it drifts to the left, and the first ship that touches it takes it.
 */
struct PowerUp
{
    PowerUpType type = PowerUpType::ExtraLife;
};

} // namespace jtype

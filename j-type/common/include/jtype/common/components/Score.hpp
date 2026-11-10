/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/Score
** File description:
** Score header file
*/

#pragma once

#include <cstdint>

namespace jtype {

/**
 * @brief The points of a player, earned by destroying Bydos.
 */
struct Score
{
    std::uint32_t points = 0;
};

} // namespace jtype

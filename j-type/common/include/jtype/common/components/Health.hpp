/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/components/Health
** File description:
** Health header file
*/

#pragma once

namespace jtype {

/**
 * @brief The health points of a ship or a Bydo: it is destroyed once current reaches 0.
 */
struct Health
{
    int current = 1;
    int max = 1;
};

} // namespace jtype

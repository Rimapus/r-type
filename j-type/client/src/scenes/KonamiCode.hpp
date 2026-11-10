/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/KonamiCode
** File description:
** KonamiCode header file
*/

#pragma once

#include <array>
#include <cstddef>

#include "input/Controls.hpp"

namespace jtype {

/**
 * @brief Finds the Konami code, up up down down left right left right, in the actions pressed in a menu.
 */
class KonamiCode
{
public:
    /**
     * @brief Give the next action pressed.
     *
     * @param action The action
     *
     * @return true when it ends the code, which then starts again from nothing.
     */
    bool enter(MenuAction action);

private:
    static constexpr std::size_t LENGTH = 8;

    // The last actions pressed, the latest at the end
    std::array<MenuAction, LENGTH> _last{};
    std::size_t _count = 0;
};

} // namespace jtype

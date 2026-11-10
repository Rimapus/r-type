/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/KonamiCode
** File description:
** KonamiCode source file
*/

#include "scenes/KonamiCode.hpp"

#include <algorithm>

namespace jtype {

namespace {

constexpr std::array<MenuAction, 8> CODE{
    MenuAction::Up,
    MenuAction::Up,
    MenuAction::Down,
    MenuAction::Down,
    MenuAction::Left,
    MenuAction::Right,
    MenuAction::Left,
    MenuAction::Right,
};

} // namespace

bool KonamiCode::enter(MenuAction action)
{
    // The last actions are compared as a whole, so a code started again in the middle of another is still found
    std::shift_left(this->_last.begin(), this->_last.end(), 1);
    this->_last.back() = action;
    this->_count = std::min(this->_count + 1, LENGTH);
    if (this->_count < LENGTH || this->_last != CODE) {
        return false;
    }
    this->_count = 0;
    return true;
}

} // namespace jtype

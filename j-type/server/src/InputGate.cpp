/*
** EPITECH PROJECT, 2026
** j-type/server/src/InputGate
** File description:
** InputGate source file
*/

#include "InputGate.hpp"

#include "jtype/common/Messages.hpp"

namespace jtype {

bool InputGate::accept(std::uint32_t tick) noexcept
{
    if (this->_tick && !isNewerTick(tick, *this->_tick)) {
        return false;
    }
    this->_tick = tick;
    return true;
}

} // namespace jtype

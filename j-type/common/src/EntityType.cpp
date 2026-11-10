/*
** EPITECH PROJECT, 2026
** j-type/common/src/EntityType
** File description:
** EntityType source file
*/

#include "jtype/common/EntityType.hpp"

namespace jtype {

EntityType getMissileType(std::uint8_t slot)
{
    return static_cast<EntityType>(static_cast<std::uint8_t>(EntityType::Missile1) + slot);
}

EntityType getPelletType(std::uint8_t slot)
{
    return static_cast<EntityType>(static_cast<std::uint8_t>(EntityType::SpreadPellet1) + slot);
}

std::optional<std::uint8_t> getPlayerSlot(EntityType type)
{
    if (type > EntityType::Player4) {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(type);
}

} // namespace jtype

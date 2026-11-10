/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/PacketError
** File description:
** PacketError source file
*/

#include "younity/network/PacketError.hpp"

namespace younity::network {

std::string_view toString(PacketError error) noexcept
{
    switch (error) {
    case PacketError::Truncated:
        return "truncated";
    case PacketError::TooLarge:
        return "too large";
    case PacketError::BadMagic:
        return "bad magic";
    case PacketError::BadVersion:
        return "bad version";
    case PacketError::UnknownType:
        return "unknown type";
    case PacketError::InvalidPayload:
        return "invalid payload";
    }
    return "unknown";
}

} // namespace younity::network

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/CodecError
** File description:
** CodecError source file
*/

#include "younity/network/CodecError.hpp"

namespace younity::network {

std::string_view toString(CodecError error) noexcept
{
    switch (error) {
    case CodecError::Truncated:
        return "truncated";
    case CodecError::TooLarge:
        return "too large";
    case CodecError::InvalidValue:
        return "invalid value";
    }
    return "unknown";
}

} // namespace younity::network

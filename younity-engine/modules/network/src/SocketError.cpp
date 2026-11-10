/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/SocketError
** File description:
** SocketError source file
*/

#include "younity/network/SocketError.hpp"

namespace younity::network {

std::string_view toString(SocketError error) noexcept
{
    switch (error) {
    case SocketError::NotOpen:
        return "not open";
    case SocketError::OpenFailed:
        return "open failed";
    case SocketError::BindFailed:
        return "bind failed";
    case SocketError::SendFailed:
        return "send failed";
    case SocketError::ReceiveFailed:
        return "receive failed";
    case SocketError::InvalidAddress:
        return "invalid address";
    case SocketError::TooLarge:
        return "too large";
    }
    return "unknown";
}

} // namespace younity::network

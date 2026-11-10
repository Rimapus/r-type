/*
** EPITECH PROJECT, 2026
** j-type/common/src/Arguments
** File description:
** Arguments source file
*/

#include "jtype/common/Arguments.hpp"

#include <charconv>
#include <system_error>

namespace jtype {

std::optional<std::uint16_t> parsePort(std::string_view text)
{
    std::uint16_t port = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), port);

    if (error != std::errc() || end != text.data() + text.size() || port == 0) {
        return std::nullopt;
    }
    return port;
}

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/FormatWithLocation
** File description:
** FormatWithLocation header file
*/

#pragma once

#include <concepts>
#include <format>
#include <source_location>
#include <string_view>

namespace younity::logger {

/**
 * @brief Format string of a log message, with the place in the code where it is written.
 *
 * A function cannot have a std::source_location default argument after its variadic arguments,
 * so the location is captured here instead: the string literal given to the logger is implicitly
 * converted to this type, and the default argument is evaluated where the logger is called.
 *
 * @tparam Args The types of the arguments of the message
 */
template <typename... Args>
class FormatWithLocation
{
public:
    /**
     * @brief Check the format string at compile time and capture where it is written.
     *        Not explicit on purpose: a string literal must convert to this type.
     *
     * @tparam String The type of the format string, e.g. a string literal
     * @param format The format string, e.g. "Entity {} created"
     * @param location Where the logger is called, captured by the default argument
     */
    template <typename String>
        requires std::convertible_to<const String &, std::string_view>
    consteval FormatWithLocation(const String &format, std::source_location location = std::source_location::current())
        : _format(format), _location(location)
    {
    }

    /**
     * @brief Get the format string.
     *
     * @return The format string, already checked against the arguments.
     */
    [[nodiscard]] constexpr const std::format_string<Args...> &getFormat(void) const noexcept
    {
        return this->_format;
    }

    /**
     * @brief Get where the logger is called.
     *
     * @return The file, line and function of the call to the logger.
     */
    [[nodiscard]] constexpr const std::source_location &getLocation(void) const noexcept
    {
        return this->_location;
    }

private:
    std::format_string<Args...> _format;
    std::source_location _location;
};

} // namespace younity::logger

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/Record
** File description:
** Record header file
*/

#pragma once

#include <chrono>
#include <source_location>
#include <string>
#include <utility>

#include "younity/logger/Level.hpp"

namespace younity::logger {

/**
 * @brief One log message and everything known about it, as given to the sinks.
 *        It cannot be modified once created.
 */
class Record
{
public:
    using Clock = std::chrono::system_clock;

    /**
     * @brief Create a log message.
     *
     * @param level The severity of the message
     * @param category The name of the logger that wrote it, e.g. "ecs"
     * @param message The message, already formatted
     * @param location Where the message was written in the code
     * @param time When the message was written
     */
    Record(Level level, std::string category, std::string message, std::source_location location, Clock::time_point time)
        : _level(level), _category(std::move(category)), _message(std::move(message)), _location(location), _time(time) {}

    /**
     * @brief Get the severity of the message.
     *
     * @return The level of the message.
     */
    [[nodiscard]] Level getLevel(void) const noexcept
    {
        return this->_level;
    }

    /**
     * @brief Get the name of the logger that wrote the message.
     *
     * @return The name of the logger, e.g. "ecs".
     */
    [[nodiscard]] const std::string &getCategory(void) const noexcept
    {
        return this->_category;
    }

    /**
     * @brief Get the text of the message.
     *
     * @return The message, already formatted.
     */
    [[nodiscard]] const std::string &getMessage(void) const noexcept
    {
        return this->_message;
    }

    /**
     * @brief Get where the message was written in the code.
     *
     * @return The file, line and function of the call to the logger.
     */
    [[nodiscard]] const std::source_location &getLocation(void) const noexcept
    {
        return this->_location;
    }

    /**
     * @brief Get when the message was written.
     *
     * @return The time of the call to the logger.
     */
    [[nodiscard]] Clock::time_point getTime(void) const noexcept
    {
        return this->_time;
    }

private:
    Level _level;
    std::string _category;
    std::string _message;
    std::source_location _location;
    Clock::time_point _time;
};

} // namespace younity::logger

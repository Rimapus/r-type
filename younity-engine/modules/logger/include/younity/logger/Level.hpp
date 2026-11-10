/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/Level
** File description:
** Level header file
*/

#pragma once

#include <cstdint>
#include <string_view>

namespace younity::logger {

/**
 * @brief Severity of a log message, from the most verbose to the most critical.
 *
 * The levels are ordered: a logger set to a level only keeps the messages of this level or above.
 * Off is not a message level, it is used as a threshold to disable a logger.
 */
enum class Level : std::uint8_t {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off,
};

/**
 * @brief Get the name of a level, as printed in the log messages.
 *
 * @param level The level to name
 *
 * @return The name in upper case, e.g. "WARN", or "UNKNOWN" for an invalid level.
 */
[[nodiscard]] constexpr std::string_view toString(Level level) noexcept
{
    switch (level) {
    case Level::Trace:
        return "TRACE";
    case Level::Debug:
        return "DEBUG";
    case Level::Info:
        return "INFO";
    case Level::Warn:
        return "WARN";
    case Level::Error:
        return "ERROR";
    case Level::Critical:
        return "CRITICAL";
    case Level::Off:
        return "OFF";
    }
    return "UNKNOWN";
}

} // namespace younity::logger

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/src/Formatter
** File description:
** Formatter source file
*/

#include "younity/logger/Formatter.hpp"
#include <chrono>
#include <ctime>
#include <format>

namespace younity::logger {

namespace {

/**
 * @brief Convert a time to the local time of the machine, daylight saving time included.
 *
 * std::chrono::current_zone() would do it in standard C++, but it is missing from the libc++ of macOS:
 * the C function is used instead, in its thread safe version, which has a different name on Windows.
 *
 * @param time The time to convert
 *
 * @return The local date and time, split into fields.
 */
std::tm toLocalTime(std::time_t time) noexcept
{
    std::tm local{};

#ifdef _WIN32
    localtime_s(&local, &time);
#else
    localtime_r(&time, &local);
#endif
    return local;
}

} // namespace

std::string formatRecord(const Record &record)
{
    const auto milliseconds = std::chrono::floor<std::chrono::milliseconds>(record.getTime());
    const auto seconds = std::chrono::floor<std::chrono::seconds>(milliseconds);
    const std::tm local = toLocalTime(Record::Clock::to_time_t(seconds));

    return std::format("[{:02}:{:02}:{:02}.{:03}] [{:<8}] [{}] {} ({}:{})",
                       local.tm_hour,
                       local.tm_min,
                       local.tm_sec,
                       (milliseconds - seconds).count(),
                       toString(record.getLevel()),
                       record.getCategory(),
                       record.getMessage(),
                       shortFileName(record.getLocation().file_name()),
                       record.getLocation().line());
}

std::string_view shortFileName(std::string_view path) noexcept
{
    const std::size_t separator = path.find_last_of("/\\");

    if (separator == std::string_view::npos) {
        return path;
    }
    return path.substr(separator + 1);
}

} // namespace younity::logger

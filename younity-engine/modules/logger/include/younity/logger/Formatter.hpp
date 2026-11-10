/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/Formatter
** File description:
** Formatter header file
*/

#pragma once

#include <string>
#include <string_view>

#include "younity/logger/Record.hpp"

namespace younity::logger {

/**
 * @brief Turn a log message into the line written by the console and file sinks.
 *
 * The line looks like "[12:03:04.123] [WARN    ] [network] Packet 12 dropped (Server.cpp:87)":
 * the time is in the local time zone of the machine with milliseconds, the level is padded to the longest level name,
 * and only the file name of the location is kept.
 *
 * @param record The message to format
 *
 * @return The formatted line, without a trailing newline.
 */
[[nodiscard]] std::string formatRecord(const Record &record);

/**
 * @brief Keep only the file name of a path, e.g. "/a/b/Registry.cpp" gives "Registry.cpp".
 *        Both '/' and '\' are separators, so it works with Linux and Windows paths.
 *
 * @param path The path to shorten
 *
 * @return The part of the path after the last separator.
 */
[[nodiscard]] std::string_view shortFileName(std::string_view path) noexcept;

} // namespace younity::logger

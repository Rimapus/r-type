/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/src/ConsoleSink
** File description:
** ConsoleSink source file
*/

#include "younity/logger/ConsoleSink.hpp"
#include "younity/logger/Formatter.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string_view>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace younity::logger {

namespace {

constexpr std::string_view RESET_COLOR = "\033[0m";

/**
 * @brief Get the ANSI escape code coloring the lines of a level.
 *
 * @param level The level of the line
 *
 * @return The escape code, e.g. red for Level::Error, or nothing for an invalid level.
 */
constexpr std::string_view colorOf(Level level) noexcept
{
    switch (level) {
    case Level::Trace:
        return "\033[90m";
    case Level::Debug:
        return "\033[36m";
    case Level::Info:
        return "\033[32m";
    case Level::Warn:
        return "\033[33m";
    case Level::Error:
        return "\033[31m";
    case Level::Critical:
        return "\033[1;41m";
    case Level::Off:
        return "";
    }
    return "";
}

/**
 * @brief Check if a standard stream is a terminal, and not a file or a pipe.
 *        On Windows, the terminal is also asked to understand the ANSI escape codes.
 *
 * @param stream The standard stream, stdout or stderr
 *
 * @return true if the stream is a terminal able to show the colors.
 */
bool isColorTerminal(std::FILE *stream) noexcept
{
#ifdef _WIN32
    if (_isatty(_fileno(stream)) == 0) {
        return false;
    }

    const HANDLE handle = GetStdHandle(stream == stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    DWORD mode = 0;

    return GetConsoleMode(handle, &mode) != 0 && SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
    return isatty(fileno(stream)) != 0;
#endif
}

/**
 * @brief Check if the colors can be used on the standard output and error output.
 *
 * @return true if both are terminals and the NO_COLOR variable is not set (https://no-color.org).
 */
bool canUseColors(void) noexcept
{
    const char *noColor = std::getenv("NO_COLOR");

    if (noColor != nullptr && noColor[0] != '\0') {
        return false;
    }
    return isColorTerminal(stdout) && isColorTerminal(stderr);
}

} // namespace

ConsoleSink::ConsoleSink(void)
    : ConsoleSink(std::cout, std::cerr, canUseColors()) {}

ConsoleSink::ConsoleSink(std::ostream &out, std::ostream &err, bool colors)
    : _out(out), _err(err), _colors(colors) {}

void ConsoleSink::write(const Record &record)
{
    std::ostream &stream = record.getLevel() >= Level::Warn ? this->_err : this->_out;

    if (this->_colors) {
        stream << colorOf(record.getLevel()) << formatRecord(record) << RESET_COLOR << '\n';
    } else {
        stream << formatRecord(record) << '\n';
    }
    // Flushed at each line: the messages keep their order between the two streams, and none is lost on a crash
    stream.flush();
}

bool ConsoleSink::hasColors(void) const noexcept
{
    return this->_colors;
}

} // namespace younity::logger

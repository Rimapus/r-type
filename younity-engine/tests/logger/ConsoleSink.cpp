/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/ConsoleSink
** File description:
** ConsoleSink unit tests
*/

#include <criterion/criterion.h>

#include <algorithm>
#include <cstdlib>
#include <source_location>
#include <sstream>
#include <string>

#include "younity/logger/ConsoleSink.hpp"
#include "younity/logger/Formatter.hpp"
#include "younity/logger/Level.hpp"
#include "younity/logger/Record.hpp"

using younity::logger::ConsoleSink;
using younity::logger::formatRecord;
using younity::logger::Level;
using younity::logger::Record;

namespace {

Record makeRecord(Level level)
{
    return Record(level, "ecs", "message", std::source_location::current(), Record::Clock::now());
}

// Writes a record of this level with colors, and returns what was written to the output and the error streams
std::string writeColored(Level level)
{
    std::ostringstream out;
    std::ostringstream err;
    ConsoleSink sink(out, err, true);

    sink.write(makeRecord(level));
    return out.str() + err.str();
}

} // namespace

Test(logger_console_sink, messages_below_warn_go_to_the_output)
{
    std::ostringstream out;
    std::ostringstream err;
    ConsoleSink sink(out, err, false);

    sink.write(makeRecord(Level::Trace));
    sink.write(makeRecord(Level::Debug));
    sink.write(makeRecord(Level::Info));
    const std::string written = out.str();

    cr_assert(err.str().empty());
    cr_assert_eq(std::ranges::count(written, '\n'), 3);
}

Test(logger_console_sink, messages_from_warn_go_to_the_error_output)
{
    std::ostringstream out;
    std::ostringstream err;
    ConsoleSink sink(out, err, false);

    sink.write(makeRecord(Level::Warn));
    sink.write(makeRecord(Level::Error));
    sink.write(makeRecord(Level::Critical));
    const std::string written = err.str();

    cr_assert(out.str().empty());
    cr_assert_eq(std::ranges::count(written, '\n'), 3);
}

Test(logger_console_sink, writes_the_formatted_line)
{
    std::ostringstream out;
    std::ostringstream err;
    ConsoleSink sink(out, err, false);
    const Record record = makeRecord(Level::Info);

    sink.write(record);
    cr_assert(out.str() == formatRecord(record) + "\n", "got \"%s\"", out.str().c_str());
}

Test(logger_console_sink, without_colors_writes_no_escape_code)
{
    std::ostringstream out;
    std::ostringstream err;
    ConsoleSink sink(out, err, false);

    sink.write(makeRecord(Level::Info));
    sink.write(makeRecord(Level::Error));
    cr_assert_eq(out.str().find('\033'), std::string::npos);
    cr_assert_eq(err.str().find('\033'), std::string::npos);
    cr_assert_not(sink.hasColors());
}

Test(logger_console_sink, colors_the_line_of_each_level)
{
    const std::string reset = "\033[0m\n";

    cr_assert(writeColored(Level::Trace).starts_with("\033[90m"));
    cr_assert(writeColored(Level::Debug).starts_with("\033[36m"));
    cr_assert(writeColored(Level::Info).starts_with("\033[32m"));
    cr_assert(writeColored(Level::Warn).starts_with("\033[33m"));
    cr_assert(writeColored(Level::Error).starts_with("\033[31m"));
    cr_assert(writeColored(Level::Critical).starts_with("\033[1;41m"));
    cr_assert(writeColored(Level::Error).ends_with(reset));
}

Test(logger_console_sink, has_colors_when_asked)
{
    std::ostringstream out;
    std::ostringstream err;
    const ConsoleSink sink(out, err, true);

    cr_assert(sink.hasColors());
}

Test(logger_console_sink, no_color_variable_disables_the_colors)
{
    setenv("NO_COLOR", "1", 1);

    const ConsoleSink sink;

    cr_assert_not(sink.hasColors());
}

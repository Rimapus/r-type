/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/Formatter
** File description:
** Formatter unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <source_location>
#include <string>

#include "younity/logger/Formatter.hpp"
#include "younity/logger/Level.hpp"
#include "younity/logger/Record.hpp"

using namespace std::chrono_literals;
using younity::logger::formatRecord;
using younity::logger::Level;
using younity::logger::Record;
using younity::logger::shortFileName;

namespace {

// 2026-10-05 12:03:04.123 UTC
const Record::Clock::time_point fixedTime = std::chrono::sys_days{std::chrono::year{2026} / 10 / 5} + 12h + 3min + 4s + 123ms;

// POSIX rule of the Paris time zone: UTC+1 in winter, UTC+2 in summer (last Sunday of March to last Sunday of October)
const char *const parisTimeZone = "CET-1CEST,M3.5.0,M10.5.0/3";

// The time is printed in local time: the tests set the time zone so they give the same result on every machine
void setTimeZone(const char *timeZone)
{
    setenv("TZ", timeZone, 1);
    tzset();
}

void useUtc(void)
{
    setTimeZone("UTC0");
}

void restoreTimeZone(void)
{
    unsetenv("TZ");
    tzset();
}

std::string expectedLocation(const std::source_location &location)
{
    return "(Formatter.cpp:" + std::to_string(location.line()) + ")";
}

} // namespace

TestSuite(logger_formatter, .init = useUtc, .fini = restoreTimeZone);

Test(logger_formatter, formats_a_full_line)
{
    const Record record(Level::Warn, "network", "Packet 12 dropped", std::source_location::current(), fixedTime);
    const std::string line = formatRecord(record);
    const std::string expected = "[12:03:04.123] [WARN    ] [network] Packet 12 dropped " + expectedLocation(record.getLocation());

    cr_assert(line == expected, "got \"%s\"", line.c_str());
}

Test(logger_formatter, pads_the_level_to_the_longest_name)
{
    const Record info(Level::Info, "ecs", "message", std::source_location::current(), fixedTime);
    const Record critical(Level::Critical, "ecs", "message", std::source_location::current(), fixedTime);

    cr_assert(formatRecord(info).find("[INFO    ]") != std::string::npos);
    cr_assert(formatRecord(critical).find("[CRITICAL]") != std::string::npos);
}

Test(logger_formatter, time_is_truncated_to_milliseconds)
{
    const Record record(Level::Info, "ecs", "message", std::source_location::current(), fixedTime + 999us);

    cr_assert(formatRecord(record).starts_with("[12:03:04.123]"));
}

Test(logger_formatter, time_is_zero_padded)
{
    const Record::Clock::time_point midnight = std::chrono::sys_days{std::chrono::year{2026} / 1 / 1};
    const Record record(Level::Info, "ecs", "message", std::source_location::current(), midnight);

    cr_assert(formatRecord(record).starts_with("[00:00:00.000]"));
}

Test(logger_formatter, time_is_in_the_local_time_zone)
{
    setTimeZone(parisTimeZone);

    const Record record(Level::Info, "ecs", "message", std::source_location::current(), fixedTime);

    // 12:03 UTC is 14:03 in Paris in October (summer time, UTC+2)
    cr_assert(formatRecord(record).starts_with("[14:03:04.123]"), "got \"%s\"", formatRecord(record).c_str());
}

Test(logger_formatter, time_follows_the_daylight_saving_time)
{
    setTimeZone(parisTimeZone);

    const Record::Clock::time_point winter = std::chrono::sys_days{std::chrono::year{2026} / 1 / 15} + 12h;
    const Record record(Level::Info, "ecs", "message", std::source_location::current(), winter);

    // 12:00 UTC is 13:00 in Paris in January (winter time, UTC+1)
    cr_assert(formatRecord(record).starts_with("[13:00:00.000]"), "got \"%s\"", formatRecord(record).c_str());
}

Test(logger_formatter, short_file_name_keeps_only_the_file)
{
    cr_assert(shortFileName("/home/user/younity/modules/ecs/src/Registry.cpp") == "Registry.cpp");
    cr_assert(shortFileName("C:\\younity\\modules\\render\\src\\Window.cpp") == "Window.cpp");
    cr_assert(shortFileName("Main.cpp") == "Main.cpp");
    cr_assert(shortFileName("").empty());
}

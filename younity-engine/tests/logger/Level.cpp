/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/Level
** File description:
** Level unit tests
*/

#include <criterion/criterion.h>

#include <string_view>

#include "younity/logger/Level.hpp"

using younity::logger::Level;
using younity::logger::toString;

Test(logger_level, every_level_has_its_name)
{
    cr_assert(toString(Level::Trace) == "TRACE");
    cr_assert(toString(Level::Debug) == "DEBUG");
    cr_assert(toString(Level::Info) == "INFO");
    cr_assert(toString(Level::Warn) == "WARN");
    cr_assert(toString(Level::Error) == "ERROR");
    cr_assert(toString(Level::Critical) == "CRITICAL");
    cr_assert(toString(Level::Off) == "OFF");
}

Test(logger_level, levels_are_ordered_by_severity)
{
    cr_assert(Level::Trace < Level::Debug);
    cr_assert(Level::Debug < Level::Info);
    cr_assert(Level::Info < Level::Warn);
    cr_assert(Level::Warn < Level::Error);
    cr_assert(Level::Error < Level::Critical);
    cr_assert(Level::Critical < Level::Off);
}

Test(logger_level, invalid_level_is_unknown)
{
    cr_assert(toString(static_cast<Level>(42)) == "UNKNOWN");
}

Test(logger_level, to_string_is_usable_at_compile_time)
{
    static_assert(toString(Level::Info) == std::string_view("INFO"));
}

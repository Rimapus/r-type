/*
** EPITECH PROJECT, 2026
** younity-engine/tests/tools/Commands
** File description:
** Commands unit tests
*/

#include <criterion/criterion.h>

#include <expected>
#include <string>

#include "younity/logger/LoggerRegistry.hpp"
#include "younity/tools/Commands.hpp"
#include "younity/tools/Console.hpp"
#include "younity/tools/MetricsStore.hpp"

using younity::logger::Level;
using younity::logger::LoggerRegistry;
using younity::tools::Console;
using younity::tools::MetricsStore;

Test(tools_commands, metrics_prints_every_metric_or_one)
{
    Console console;
    MetricsStore metrics;
    LoggerRegistry logs;

    younity::tools::registerCommands(console, metrics, logs);
    cr_assert_eq(*console.execute("metrics"), "No metric");
    metrics.record("fps", 60.0);
    metrics.record("frame_time", 2.0);
    cr_assert_eq(*console.execute("metrics fps"), "fps = 60 (average 60)");
    cr_assert_eq(*console.execute("metrics"), "fps = 60 (average 60)\nframe_time = 2 (average 2)");
    cr_assert_not(console.execute("metrics missing").has_value());
}

Test(tools_commands, log_level_changes_the_level_of_a_logger)
{
    Console console;
    MetricsStore metrics;
    LoggerRegistry logs;

    younity::tools::registerCommands(console, metrics, logs);
    cr_assert_eq(*console.execute("log.level network DEBUG"), "network logs from DEBUG");
    cr_assert(logs.get("network").getLevel() == Level::Debug);
    cr_assert_not(console.execute("log.level network LOUD").has_value());
    cr_assert_not(console.execute("log.level network").has_value());
}

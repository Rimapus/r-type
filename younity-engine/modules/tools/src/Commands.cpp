/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/src/Commands
** File description:
** Commands source file
*/

#include "younity/tools/Commands.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "younity/logger/Level.hpp"
#include "younity/tools/Metric.hpp"

namespace younity::tools {

namespace {

/**
 * @brief Write a metric on one line of the console.
 *
 * @param metric The metric
 *
 * @return Its name, its last value and its average.
 */
std::string describe(const Metric &metric)
{
    return std::format("{} = {} (average {})", metric.name, metric.last, metric.average);
}

/**
 * @brief Find the level written in a command, e.g. "DEBUG".
 *
 * @param name The name of the level, as logger::toString() writes it
 *
 * @return The level, or nothing if no level has this name.
 */
std::optional<logger::Level> parseLevel(std::string_view name)
{
    for (auto value = static_cast<std::uint8_t>(logger::Level::Trace); value <= static_cast<std::uint8_t>(logger::Level::Off);
         ++value) {
        const auto level = static_cast<logger::Level>(value);

        if (logger::toString(level) == name) {
            return level;
        }
    }
    return std::nullopt;
}

} // namespace

void registerCommands(Console &console, MetricsStore &metrics, logger::LoggerRegistry &logs)
{
    console.addCommand("metrics", "Print the last value and the average of every metric, or of the one named",
                       [&metrics](std::span<const std::string_view> arguments) -> std::expected<std::string, ConsoleError> {
                           if (arguments.size() > 1) {
                               return std::unexpected(ConsoleError{"metrics takes at most one name"});
                           }
                           if (arguments.size() == 1) {
                               const std::optional<Metric> metric = metrics.getMetric(arguments.front());

                               if (!metric) {
                                   return std::unexpected(ConsoleError{"no metric named " + std::string(arguments.front())});
                               }
                               return describe(*metric);
                           }
                           const std::vector<Metric> all = metrics.getMetrics();
                           std::string text;

                           for (const Metric &metric : all) {
                               text += (text.empty() ? "" : "\n") + describe(metric);
                           }
                           return all.empty() ? std::string("No metric") : text;
                       });
    console.addCommand("log.level", "Set the level of a logger: log.level <logger> <TRACE|DEBUG|INFO|WARN|ERROR|CRITICAL|OFF>",
                       [&logs](std::span<const std::string_view> arguments) -> std::expected<std::string, ConsoleError> {
                           if (arguments.size() != 2) {
                               return std::unexpected(ConsoleError{"log.level takes a logger and a level"});
                           }
                           const std::optional<logger::Level> level = parseLevel(arguments[1]);

                           if (!level) {
                               return std::unexpected(ConsoleError{"unknown level " + std::string(arguments[1])});
                           }
                           logs.setLevel(arguments[0], *level);
                           return std::format("{} logs from {}", arguments[0], logger::toString(*level));
                       });
}

} // namespace younity::tools

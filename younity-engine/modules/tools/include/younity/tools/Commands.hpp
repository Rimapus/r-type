/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/include/younity/tools/Commands
** File description:
** Commands header file
*/

#pragma once

#include "younity/logger/LoggerRegistry.hpp"
#include "younity/tools/Console.hpp"
#include "younity/tools/MetricsStore.hpp"

namespace younity::tools {

/**
 * @brief Add the commands of the engine to a console: `metrics` prints the last value and the average of every metric,
 *        or of the one named after it, and `log.level <logger> <level>` changes the level of a logger, e.g.
 *        `log.level network DEBUG`.
 *
 * @param console The console receiving the commands, e.g. tools::console()
 * @param metrics The metrics `metrics` reads, e.g. tools::metrics(); it must outlive the commands
 * @param logs The loggers `log.level` changes, e.g. logger::registry(); it must outlive the commands
 *
 * @throw std::invalid_argument if one of the names is already used.
 */
void registerCommands(Console &console, MetricsStore &metrics, logger::LoggerRegistry &logs);

} // namespace younity::tools

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/Global
** File description:
** Global header file
*/

#pragma once

#include <filesystem>
#include <string_view>

#include "younity/logger/Logger.hpp"
#include "younity/logger/LoggerRegistry.hpp"

namespace younity::logger {

/**
 * @brief Get the registry shared by the whole engine and the game.
 *        It is created at its first use, which is safe from several threads,
 *        with a ConsoleSink named "console" so the messages are shown without any setup.
 *        If the YOUNITY_LOG_FILE variable is set, a FileSink named "file" writes to this path too,
 *        e.g. "YOUNITY_LOG_FILE=game.log ./game" to get the logs without rebuilding.
 *
 * @return The global registry, e.g. to set the levels or add a sink.
 */
[[nodiscard]] LoggerRegistry &registry(void);

/**
 * @brief Get a logger of the global registry, created if it does not exist yet.
 *
 * @param name The name of the logger, e.g. "game"
 *
 * @return The logger, valid until the end of the program.
 */
[[nodiscard]] Logger &get(std::string_view name);

/**
 * @brief Write the messages of every logger to a file too, replacing the file already enabled.
 *
 * @param path The path of the log file, e.g. "logs/game.log"
 * @param append Whether the messages are added after the content of the file, instead of replacing it
 *
 * @throw std::runtime_error if the file cannot be opened.
 */
void enableFile(const std::filesystem::path &path, bool append = false);

/**
 * @brief Stop writing the messages to the file, and close it.
 *
 * @return true if a file was enabled.
 */
bool disableFile(void);

} // namespace younity::logger

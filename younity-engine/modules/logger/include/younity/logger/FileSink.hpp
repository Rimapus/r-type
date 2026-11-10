/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/FileSink
** File description:
** FileSink header file
*/

#pragma once

#include <filesystem>
#include <fstream>

#include "younity/logger/ISink.hpp"
#include "younity/logger/Record.hpp"

namespace younity::logger {

/**
 * @brief Sink writing the log messages to a file, one line per message, without colors.
 *
 * The lines are buffered to keep the logging fast, but the Level::Error and Level::Critical ones
 * are written to the disk at once, so they are kept if the program crashes right after.
 */
class FileSink : public ISink
{
public:
    /**
     * @brief Open the log file, and create its missing folders.
     *
     * @param path The path of the log file, e.g. "logs/game.log"
     * @param append Whether the messages are added after the content of the file, instead of replacing it
     *
     * @throw std::runtime_error if the file cannot be opened.
     */
    explicit FileSink(const std::filesystem::path &path, bool append = false);

    /**
     * @brief Write the formatted line of a log message, followed by a newline.
     *        A failed write, e.g. when the disk is full, is reported once on the error output.
     *
     * @param record The message to write
     */
    void write(const Record &record) override;

    /**
     * @brief Get the path of the log file.
     *
     * @return The path given at the creation.
     */
    [[nodiscard]] const std::filesystem::path &getPath(void) const noexcept;

private:
    std::filesystem::path _path;
    std::ofstream _file;
    bool _failed = false;
};

} // namespace younity::logger

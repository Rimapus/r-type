/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/ConsoleSink
** File description:
** ConsoleSink header file
*/

#pragma once

#include <ostream>

#include "younity/logger/ISink.hpp"
#include "younity/logger/Record.hpp"

namespace younity::logger {

/**
 * @brief Sink writing the log messages to the terminal, one colored line per message.
 *
 * The messages below Level::Warn go to the standard output, the others to the error output,
 * so the errors can be redirected on their own, e.g. "./game 2> errors.log".
 */
class ConsoleSink : public ISink
{
public:
    /**
     * @brief Create a sink writing to std::cout and std::cerr.
     *        The colors are used only if both are terminals and the NO_COLOR variable is not set,
     *        so no escape code is written in a file or a pipe.
     */
    ConsoleSink(void);

    /**
     * @brief Create a sink writing to the given streams, e.g. string streams in the tests.
     *
     * @param out Where the messages below Level::Warn are written, it must outlive the sink
     * @param err Where the other messages are written, it must outlive the sink
     * @param colors Whether each line is colored with ANSI escape codes
     */
    ConsoleSink(std::ostream &out, std::ostream &err, bool colors);

    /**
     * @brief Write the formatted line of a log message, followed by a newline.
     *
     * @param record The message to write
     */
    void write(const Record &record) override;

    /**
     * @brief Check if the lines are colored.
     *
     * @return true if each line is colored with ANSI escape codes.
     */
    [[nodiscard]] bool hasColors(void) const noexcept;

private:
    std::ostream &_out;
    std::ostream &_err;
    bool _colors;
};

} // namespace younity::logger

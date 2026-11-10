/*
** EPITECH PROJECT, 2026
** j-type/server/src/ConsoleInput
** File description:
** ConsoleInput header file
*/

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "younity/network/ThreadSafeQueue.hpp"

namespace jtype {

// The lines that wait for the game thread at most: a line typed past them is dropped
inline constexpr std::size_t MAX_CONSOLE_LINES = 64;

/**
 * @brief Reads one line of input, waiting for it.
 *
 * @return The line without its end, or nothing once the input is over.
 */
using LineReader = std::function<std::optional<std::string>(void)>;

/**
 * @brief Read a line of the standard input, the terminal of the server.
 *
 * @return The line, or nothing once the standard input is closed.
 */
[[nodiscard]] std::optional<std::string> readStandardInput(void);

/**
 * @brief Reads the lines typed in the terminal of the server on a thread of its own, so the game loop never waits for
 *        them, and hands them to the game thread.
 *
 * The thread is detached: a read blocked on the terminal cannot be interrupted on every system, so the thread ends
 * when the input is over, or with the process. It holds only the reader and the queue it shares with this object,
 * never the server.
 */
class ConsoleInput
{
public:
    /**
     * @brief Start the thread that reads the lines.
     *
     * @param readLine How to read a line, the standard input by default
     */
    explicit ConsoleInput(LineReader readLine = readStandardInput);

    /**
     * @brief Take the oldest line read, without waiting.
     *
     * @return The line, or nothing while none is waiting.
     */
    [[nodiscard]] std::optional<std::string> takeLine(void);

private:
    std::shared_ptr<younity::network::ThreadSafeQueue<std::string>> _lines;
};

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/include/younity/editor/ConsoleWindow
** File description:
** ConsoleWindow header file
*/

#pragma once

#include <cstddef>
#include <memory>
#include <optional>

#include "younity/core/Rect.hpp"

namespace younity::logger {
class LoggerRegistry;
}

namespace younity::tools {
class Console;
}

namespace younity::editor {

/**
 * @brief Title of the console window, also its id in the debug interface.
 */
inline constexpr const char *CONSOLE_TITLE = "Console";

/**
 * @brief The developer console drawn over the game: an input line, a scrollback with the log messages and
 *        the command results, a command history and a name completion.
 *
 * The window captures the messages of a logger registry through its own sink. The sink is thread safe,
 * so modules can log from their own threads; draw() and the console itself are used from the game loop.
 * The registry must outlive the window, which has at most one console window per registry.
 */
class ConsoleWindow
{
public:
    /**
     * @brief Create a console window, capturing the log messages of a registry.
     *
     * @param console The variables and commands to show and run
     * @param logs The registry whose messages are added to the scrollback
     * @param maxLines The maximum number of lines kept in the scrollback
     *
     * @throw std::invalid_argument if maxLines is zero, or if another console window already captures
     *        the messages of this registry.
     */
    explicit ConsoleWindow(tools::Console &console, logger::LoggerRegistry &logs, std::size_t maxLines = 1000);

    /**
     * @brief Remove the log sink and destroy the window.
     */
    ~ConsoleWindow(void);

    ConsoleWindow(const ConsoleWindow &) = delete;
    ConsoleWindow &operator=(const ConsoleWindow &) = delete;

    /**
     * @brief Draw the console window. Called once per frame between window.clear() and window.display().
     *
     * Enter runs the input line, Up and Down recall the command history, and Tab completes a command
     * or variable name. The logs written after the window was created are shown in the scrollback.
     *
     * @param area Where the window is fixed, in pixels from the top left corner of the window, e.g. by a Workspace, or
     *        nothing to let the mouse move and resize it
     */
    void draw(const std::optional<Rectf> &area = std::nullopt);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace younity::editor

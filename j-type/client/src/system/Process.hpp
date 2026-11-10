/*
** EPITECH PROJECT, 2026
** j-type/client/src/system/Process
** File description:
** Process header file
*/

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace jtype {

/**
 * @brief Another program, started without waiting for it, like J-Climb from the main menu.
 */
class Process
{
public:
    Process(void) = default;

    /**
     * @brief Forget the program without stopping it: it keeps running once the client closes.
     */
    ~Process(void);

    Process(const Process &) = delete;
    Process &operator=(const Process &) = delete;
    Process(Process &&) = delete;
    Process &operator=(Process &&) = delete;

    /**
     * @brief Start a program, unless the last one started still runs.
     *
     * @param program Its file, relative to the current folder or absolute
     * @param arguments Its arguments, after its name
     *
     * @return true if the program started, false if the last one still runs or the program cannot be started.
     */
    bool start(const std::filesystem::path &program, const std::vector<std::string> &arguments);

    /**
     * @brief Tell if the last program started still runs.
     *
     * @return true until it ends.
     */
    [[nodiscard]] bool isRunning(void);

private:
    // The process id on Linux and macOS, the handle of the process on Windows; 0 for none
    std::intptr_t _process = 0;
};

} // namespace jtype

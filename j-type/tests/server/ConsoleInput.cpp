/*
** EPITECH PROJECT, 2026
** j-type/tests/server/ConsoleInput
** File description:
** ConsoleInput unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "ConsoleInput.hpp"

using jtype::ConsoleInput;

namespace {

// Read the lines of a text, kept alive by the reader: the thread may outlive the test
jtype::LineReader readText(const std::string &text)
{
    auto input = std::make_shared<std::istringstream>(text);

    return [input]() -> std::optional<std::string> {
        std::string line;

        if (!std::getline(*input, line)) {
            return std::nullopt;
        }
        return line;
    };
}

// Take the lines read, waiting at most a second for the thread to read them all
std::vector<std::string> takeLines(ConsoleInput &console, std::size_t count)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    std::vector<std::string> lines;

    while (lines.size() < count && std::chrono::steady_clock::now() < deadline) {
        if (std::optional<std::string> line = console.takeLine()) {
            lines.push_back(*line);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    return lines;
}

} // namespace

Test(server_console_input, hands_the_lines_in_order)
{
    ConsoleInput console(readText("players\nkick 2\nwaves\n"));

    cr_assert(takeLines(console, 3) == (std::vector<std::string>{"players", "kick 2", "waves"}));
}

Test(server_console_input, gives_nothing_while_no_line_was_typed)
{
    ConsoleInput console(readText(""));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    cr_assert_not(console.takeLine().has_value());
}

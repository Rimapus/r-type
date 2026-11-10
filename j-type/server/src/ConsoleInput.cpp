/*
** EPITECH PROJECT, 2026
** j-type/server/src/ConsoleInput
** File description:
** ConsoleInput source file
*/

#include "ConsoleInput.hpp"

#include <iostream>
#include <thread>
#include <utility>

namespace jtype {

std::optional<std::string> readStandardInput(void)
{
    std::string line;

    if (!std::getline(std::cin, line)) {
        return std::nullopt;
    }
    return line;
}

ConsoleInput::ConsoleInput(LineReader readLine)
    : _lines(std::make_shared<younity::network::ThreadSafeQueue<std::string>>(MAX_CONSOLE_LINES))
{
    std::thread([lines = this->_lines, readLine = std::move(readLine)] {
        while (std::optional<std::string> line = readLine()) {
            (void)lines->push(std::move(*line));
        }
    }).detach();
}

std::optional<std::string> ConsoleInput::takeLine(void)
{
    return this->_lines->tryPop();
}

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/src/Logger
** File description:
** Logger source file
*/

#include "younity/logger/Logger.hpp"
#include "younity/logger/Record.hpp"

namespace younity::logger {

Logger::Logger(std::string name, ISink &sink, Level level)
    : _name(std::move(name)), _sink(sink), _level(level) {}

const std::string &Logger::getName(void) const noexcept
{
    return this->_name;
}

// Relaxed order: the level is a lone value, no other data is published with it
Level Logger::getLevel(void) const noexcept
{
    return this->_level.load(std::memory_order_relaxed);
}

void Logger::setLevel(Level level) noexcept
{
    this->_level.store(level, std::memory_order_relaxed);
}

bool Logger::isEnabled(Level level) const noexcept
{
    return level != Level::Off && level >= this->getLevel();
}

void Logger::write(Level level, std::string message, const std::source_location &location)
{
    this->_sink.write(Record(level, this->_name, std::move(message), location, Record::Clock::now()));
}

} // namespace younity::logger

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/src/LoggerRegistry
** File description:
** LoggerRegistry source file
*/

#include "younity/logger/LoggerRegistry.hpp"
#include <utility>

namespace younity::logger {

LoggerRegistry::LoggerRegistry(Level level)
    : _initialLevel(level), _level(level) {}

Logger &LoggerRegistry::get(std::string_view name)
{
    const std::lock_guard lock(this->_mutex);
    const auto it = this->_loggers.find(name);

    if (it != this->_loggers.end()) {
        return *it->second;
    }

    std::string key(name);
    auto logger = std::make_unique<Logger>(key, this->_dispatcher, this->_level);

    return *this->_loggers.emplace(std::move(key), std::move(logger)).first->second;
}

void LoggerRegistry::setLevel(Level level)
{
    const std::lock_guard lock(this->_mutex);

    this->_level = level;
    for (const auto &entry : this->_loggers) {
        entry.second->setLevel(level);
    }
}

void LoggerRegistry::setLevel(std::string_view name, Level level)
{
    this->get(name).setLevel(level);
}

void LoggerRegistry::addSink(std::string name, std::unique_ptr<ISink> sink)
{
    this->_dispatcher.addSink(std::move(name), std::move(sink));
}

bool LoggerRegistry::removeSink(std::string_view name)
{
    return this->_dispatcher.removeSink(name);
}

bool LoggerRegistry::hasSink(std::string_view name) const
{
    return this->_dispatcher.hasSink(name);
}

void LoggerRegistry::reset(void)
{
    this->_dispatcher.clear();
    this->setLevel(this->_initialLevel);
}

} // namespace younity::logger

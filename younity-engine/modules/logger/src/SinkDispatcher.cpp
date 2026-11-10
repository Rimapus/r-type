/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/src/SinkDispatcher
** File description:
** SinkDispatcher source file
*/

#include "younity/logger/SinkDispatcher.hpp"
#include <algorithm>
#include <stdexcept>

namespace younity::logger {

void SinkDispatcher::write(const Record &record)
{
    const std::lock_guard lock(this->_mutex);

    for (const auto &entry : this->_sinks) {
        entry.second->write(record);
    }
}

void SinkDispatcher::addSink(std::string name, std::unique_ptr<ISink> sink)
{
    if (sink == nullptr) {
        throw std::invalid_argument("younity::logger::SinkDispatcher: the sink is null");
    }

    const std::lock_guard lock(this->_mutex);
    const auto it = std::ranges::find_if(this->_sinks, [&name](const auto &entry) { return entry.first == name; });

    if (it != this->_sinks.end()) {
        it->second = std::move(sink);
        return;
    }
    this->_sinks.emplace_back(std::move(name), std::move(sink));
}

bool SinkDispatcher::removeSink(std::string_view name)
{
    const std::lock_guard lock(this->_mutex);

    return std::erase_if(this->_sinks, [name](const auto &entry) { return entry.first == name; }) > 0;
}

bool SinkDispatcher::hasSink(std::string_view name) const
{
    const std::lock_guard lock(this->_mutex);

    return std::ranges::any_of(this->_sinks, [name](const auto &entry) { return entry.first == name; });
}

void SinkDispatcher::clear(void)
{
    const std::lock_guard lock(this->_mutex);

    this->_sinks.clear();
}

} // namespace younity::logger

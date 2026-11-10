/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/src/ScopedTimer
** File description:
** ScopedTimer source file
*/

#include "younity/tools/ScopedTimer.hpp"
#include <stdexcept>
#include <utility>

namespace younity::tools {

ScopedTimer::ScopedTimer(MetricsStore &store, std::string name)
    : _store(store), _name(std::move(name))
{
    if (this->_name.empty()) {
        throw std::invalid_argument("younity::tools::ScopedTimer: the name of a timer cannot be empty");
    }
}

ScopedTimer::~ScopedTimer(void)
{
    this->stop();
}

void ScopedTimer::stop(void)
{
    if (this->_stopped) {
        return;
    }
    this->_stopped = true;
    this->_store.record(this->_name, static_cast<double>(this->_clock.getElapsedTime()), MetricKind::Timer);
}

} // namespace younity::tools

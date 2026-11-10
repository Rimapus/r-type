/*
** EPITECH PROJECT, 2026
** j-type/server/src/DropLog
** File description:
** DropLog source file
*/

#include "DropLog.hpp"

#include <format>
#include <utility>

namespace jtype {

DropLog::DropLog(std::chrono::milliseconds interval) noexcept
    : _interval(interval)
{
}

std::string DropLog::reject(std::string_view reason, std::chrono::steady_clock::time_point now)
{
    if (this->_count == 0) {
        this->_firstReason = reason;
    }
    ++this->_count;
    if (now - this->_lastReport < this->_interval) {
        return {};
    }
    std::string line = std::format("Dropped {} invalid datagrams, e.g. {}", this->_count, this->_firstReason);

    this->_count = 0;
    this->_firstReason.clear();
    this->_lastReport = now;
    return line;
}

} // namespace jtype

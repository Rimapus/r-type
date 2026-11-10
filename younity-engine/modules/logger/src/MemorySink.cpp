/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/src/MemorySink
** File description:
** MemorySink source file
*/

#include "younity/logger/MemorySink.hpp"

namespace younity::logger {

void MemorySink::write(const Record &record)
{
    this->_records.push_back(record);
}

const std::vector<Record> &MemorySink::getRecords(void) const noexcept
{
    return this->_records;
}

void MemorySink::clear(void) noexcept
{
    this->_records.clear();
}

} // namespace younity::logger

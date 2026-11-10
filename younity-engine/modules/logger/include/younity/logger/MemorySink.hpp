/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/MemorySink
** File description:
** MemorySink header file
*/

#pragma once

#include <vector>

#include "younity/logger/ISink.hpp"
#include "younity/logger/Record.hpp"

namespace younity::logger {

/**
 * @brief Sink keeping the log messages in memory, to read them back in the tests.
 */
class MemorySink : public ISink
{
public:
    /**
     * @brief Keep a copy of the log message.
     *
     * @param record The message to keep
     */
    void write(const Record &record) override;

    /**
     * @brief Get the kept log messages.
     *
     * @return The messages, from the oldest to the newest.
     */
    [[nodiscard]] const std::vector<Record> &getRecords(void) const noexcept;

    /**
     * @brief Remove all the kept log messages.
     */
    void clear(void) noexcept;

private:
    std::vector<Record> _records;
};

} // namespace younity::logger

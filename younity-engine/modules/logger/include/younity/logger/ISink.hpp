/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/ISink
** File description:
** ISink header file
*/

#pragma once

#include "younity/logger/Record.hpp"

namespace younity::logger {

/**
 * @brief Destination of the log messages: the console, a file, the memory...
 */
class ISink
{
public:
    virtual ~ISink(void) = default;

    /**
     * @brief Write a log message to the destination.
     *
     * @param record The message to write
     */
    virtual void write(const Record &record) = 0;
};

} // namespace younity::logger

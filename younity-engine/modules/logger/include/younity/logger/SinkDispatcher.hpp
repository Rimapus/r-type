/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/SinkDispatcher
** File description:
** SinkDispatcher header file
*/

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "younity/logger/ISink.hpp"
#include "younity/logger/Record.hpp"

namespace younity::logger {

/**
 * @brief Sink owning named sinks, e.g. "console" and "file", and writing each message to all of them.
 *
 * A mutex makes it safe to use from several threads: the messages are never mixed,
 * and the sinks can be added or removed while other threads are logging.
 * The sinks must not log themselves, or they would wait forever for the mutex.
 */
class SinkDispatcher : public ISink
{
public:
    /**
     * @brief Write a log message to every sink, in the order they were added.
     *
     * @param record The message to write
     */
    void write(const Record &record) override;

    /**
     * @brief Add a sink, replacing the one with the same name.
     *
     * @param name The name of the sink, used to remove it, e.g. "file"
     * @param sink The sink to add
     *
     * @throw std::invalid_argument if the sink is null.
     */
    void addSink(std::string name, std::unique_ptr<ISink> sink);

    /**
     * @brief Remove and destroy a sink, does nothing if there is no sink with this name.
     *
     * @param name The name of the sink to remove
     *
     * @return true if a sink has been removed.
     */
    bool removeSink(std::string_view name);

    /**
     * @brief Check if there is a sink with this name.
     *
     * @param name The name of the sink
     *
     * @return true if a sink has this name.
     */
    [[nodiscard]] bool hasSink(std::string_view name) const;

    /**
     * @brief Remove and destroy all the sinks.
     */
    void clear(void);

private:
    mutable std::mutex _mutex;
    std::vector<std::pair<std::string, std::unique_ptr<ISink>>> _sinks;
};

} // namespace younity::logger

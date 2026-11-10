/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/LoggerRegistry
** File description:
** LoggerRegistry header file
*/

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "younity/logger/ISink.hpp"
#include "younity/logger/Level.hpp"
#include "younity/logger/Logger.hpp"
#include "younity/logger/SinkDispatcher.hpp"

namespace younity::logger {

/**
 * @brief Creates the named loggers on demand, sets their levels, and owns the sinks they all write to.
 *
 * A logger is never destroyed before the registry: the references returned by get stay valid,
 * even after a reset. All the methods can be called from several threads.
 */
class LoggerRegistry
{
public:
    /**
     * @brief Create a registry without any logger nor sink.
     *
     * @param level The level of the new loggers, and of all the loggers after a reset
     */
    explicit LoggerRegistry(Level level = Level::Info);

    LoggerRegistry(const LoggerRegistry &) = delete;
    LoggerRegistry &operator=(const LoggerRegistry &) = delete;

    /**
     * @brief Get the logger of this name, created if it does not exist yet.
     *
     * @param name The name of the logger, e.g. "ecs"
     *
     * @return The logger, valid as long as the registry.
     */
    [[nodiscard]] Logger &get(std::string_view name);

    /**
     * @brief Set the level of every logger, and of the loggers created after.
     *
     * @param level The new level
     */
    void setLevel(Level level);

    /**
     * @brief Set the level of one logger, created if it does not exist yet.
     *
     * @param name The name of the logger, e.g. "network"
     * @param level The new level
     */
    void setLevel(std::string_view name, Level level);

    /**
     * @brief Add a sink written by every logger, replacing the one with the same name.
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
     * @brief Remove all the sinks and set every logger back to the level given at the creation.
     *        The loggers are kept, so the references to them stay valid.
     */
    void reset(void);

private:
    const Level _initialLevel;
    Level _level;
    std::mutex _mutex;
    // Declared before the loggers, which write to it: it is destroyed after them
    SinkDispatcher _dispatcher;
    std::map<std::string, std::unique_ptr<Logger>, std::less<>> _loggers;
};

} // namespace younity::logger

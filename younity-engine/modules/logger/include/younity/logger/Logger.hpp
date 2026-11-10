/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/include/younity/logger/Logger
** File description:
** Logger header file
*/

#pragma once

#include <atomic>
#include <format>
#include <source_location>
#include <string>
#include <type_traits>
#include <utility>

#include "younity/logger/FormatWithLocation.hpp"
#include "younity/logger/ISink.hpp"
#include "younity/logger/Level.hpp"

namespace younity::logger {

/**
 * @brief Named logger of one part of the engine or of the game, e.g. "ecs" or "network".
 *
 * Each message is written with a level: the messages below the level of the logger are dropped
 * before being formatted, so a filtered message only costs a comparison.
 * The format string is checked at compile time against the arguments, like std::format.
 */
class Logger
{
public:
    /**
     * @brief Create a logger.
     *
     * @param name The name of the logger, written in each message, e.g. "ecs"
     * @param sink Where the messages are written, it must outlive the logger
     * @param level The lowest level of the written messages
     */
    Logger(std::string name, ISink &sink, Level level = Level::Info);

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    /**
     * @brief Get the name of the logger.
     *
     * @return The name of the logger, e.g. "ecs".
     */
    [[nodiscard]] const std::string &getName(void) const noexcept;

    /**
     * @brief Get the lowest level of the written messages.
     *
     * @return The level of the logger.
     */
    [[nodiscard]] Level getLevel(void) const noexcept;

    /**
     * @brief Set the lowest level of the written messages, Level::Off to write none.
     *        It can be called while other threads are logging.
     *
     * @param level The new lowest level
     */
    void setLevel(Level level) noexcept;

    /**
     * @brief Check if a message of this level would be written.
     *
     * @param level The level of the message
     *
     * @return true if the level is at least the level of the logger, never for Level::Off.
     */
    [[nodiscard]] bool isEnabled(Level level) const noexcept;

    /**
     * @brief Write a message of the given level, if it is enabled.
     *
     * @tparam Args The types of the arguments of the message
     * @param level The level of the message
     * @param format The format string, e.g. "Entity {} created"
     * @param args The arguments of the format string
     */
    template <typename... Args>
    void log(Level level, FormatWithLocation<std::type_identity_t<Args>...> format, Args &&...args)
    {
        if (!this->isEnabled(level)) {
            return;
        }
        this->write(level, std::format(format.getFormat(), std::forward<Args>(args)...), format.getLocation());
    }

    /**
     * @brief Write a Level::Trace message: the very detailed steps, e.g. each system update.
     *
     * @tparam Args The types of the arguments of the message
     * @param format The format string, e.g. "Entity {} created"
     * @param args The arguments of the format string
     */
    template <typename... Args>
    void trace(FormatWithLocation<std::type_identity_t<Args>...> format, Args &&...args)
    {
        this->log(Level::Trace, std::move(format), std::forward<Args>(args)...);
    }

    /**
     * @brief Write a Level::Debug message: what helps to debug, e.g. an entity created.
     *
     * @tparam Args The types of the arguments of the message
     * @param format The format string, e.g. "Entity {} created"
     * @param args The arguments of the format string
     */
    template <typename... Args>
    void debug(FormatWithLocation<std::type_identity_t<Args>...> format, Args &&...args)
    {
        this->log(Level::Debug, std::move(format), std::forward<Args>(args)...);
    }

    /**
     * @brief Write a Level::Info message: the normal steps, e.g. a client connected.
     *
     * @tparam Args The types of the arguments of the message
     * @param format The format string, e.g. "Entity {} created"
     * @param args The arguments of the format string
     */
    template <typename... Args>
    void info(FormatWithLocation<std::type_identity_t<Args>...> format, Args &&...args)
    {
        this->log(Level::Info, std::move(format), std::forward<Args>(args)...);
    }

    /**
     * @brief Write a Level::Warn message: something unexpected, but handled, e.g. a packet dropped.
     *
     * @tparam Args The types of the arguments of the message
     * @param format The format string, e.g. "Entity {} created"
     * @param args The arguments of the format string
     */
    template <typename... Args>
    void warn(FormatWithLocation<std::type_identity_t<Args>...> format, Args &&...args)
    {
        this->log(Level::Warn, std::move(format), std::forward<Args>(args)...);
    }

    /**
     * @brief Write a Level::Error message: something failed, e.g. a texture not found.
     *
     * @tparam Args The types of the arguments of the message
     * @param format The format string, e.g. "Entity {} created"
     * @param args The arguments of the format string
     */
    template <typename... Args>
    void error(FormatWithLocation<std::type_identity_t<Args>...> format, Args &&...args)
    {
        this->log(Level::Error, std::move(format), std::forward<Args>(args)...);
    }

    /**
     * @brief Write a Level::Critical message: the engine cannot continue, e.g. no window.
     *
     * @tparam Args The types of the arguments of the message
     * @param format The format string, e.g. "Entity {} created"
     * @param args The arguments of the format string
     */
    template <typename... Args>
    void critical(FormatWithLocation<std::type_identity_t<Args>...> format, Args &&...args)
    {
        this->log(Level::Critical, std::move(format), std::forward<Args>(args)...);
    }

private:
    /**
     * @brief Create the record of a formatted message and give it to the sink.
     *
     * @param level The level of the message
     * @param message The formatted message
     * @param location Where the logger is called
     */
    void write(Level level, std::string message, const std::source_location &location);

    std::string _name;
    ISink &_sink;
    std::atomic<Level> _level;
};

} // namespace younity::logger

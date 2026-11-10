/*
** EPITECH PROJECT, 2026
** younity-engine/modules/plugin/include/younity/plugin/PluginError
** File description:
** PluginError header file
*/

#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

namespace younity::plugin {

/**
 * @brief Why a plugin or a plugin configuration could not be loaded.
 */
enum class PluginErrorCode {
    /** @brief The plugin file or the configuration file does not exist. */
    NotFound,
    /** @brief The library or the configuration file cannot be opened. */
    OpenFailed,
    /** @brief The library does not export the descriptor function. */
    MissingDescriptor,
    /** @brief The ABI version of the plugin is not the one of the engine. */
    AbiMismatch,
    /** @brief The descriptor has a null name, version or callback. */
    InvalidDescriptor,
    /** @brief The configuration is not valid JSON or does not have the expected shape. */
    InvalidConfig,
};

/**
 * @brief Get a short human-readable description of a plugin error code.
 *
 * @param code The code to describe
 *
 * @return The description, e.g. "missing descriptor".
 */
[[nodiscard]] std::string_view toString(PluginErrorCode code) noexcept;

/**
 * @brief What went wrong while loading a plugin or its configuration, and where.
 */
struct PluginError
{
    /** @brief The reason. */
    PluginErrorCode code = PluginErrorCode::OpenFailed;
    /** @brief The plugin file or the configuration file. */
    std::filesystem::path path;
    /** @brief The line of a JSON syntax error, or 0 when the error has no line. */
    std::size_t line = 0;
    /** @brief The reason in a sentence, ready to show to the user. */
    std::string message;
};

} // namespace younity::plugin

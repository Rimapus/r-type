/*
** EPITECH PROJECT, 2026
** younity-engine/modules/plugin/include/younity/plugin/Plugin
** File description:
** Plugin header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

#include "younity/plugin/PluginAPI.hpp"
#include "younity/plugin/PluginError.hpp"

namespace younity::plugin {

/**
 * @brief One shared library loaded at run time, with the instance of its plugin.
 *
 * loadFromFile opens the library, reads its descriptor and creates the context of the
 * plugin, but does not start it. start(), update() and stop() call the callbacks of the
 * descriptor; unload(), the destructor included, stops the plugin, destroys its context
 * and closes the library.
 *
 * A Plugin is move-only: a move keeps the library loaded, and the moved-from object owns
 * nothing anymore.
 */
class Plugin
{
public:
    /**
     * @brief Unload the plugin: stop it, destroy its context and close the library.
     */
    ~Plugin(void);

    Plugin(const Plugin &) = delete;
    Plugin &operator=(const Plugin &) = delete;

    /**
     * @brief Take the library, the descriptor and the context of another plugin.
     *
     * @param other The plugin to take from, left empty
     */
    Plugin(Plugin &&other) noexcept;

    /**
     * @brief Unload the current plugin, then take the one of another plugin.
     *
     * @param other The plugin to take from, left empty
     *
     * @return This plugin.
     */
    Plugin &operator=(Plugin &&other) noexcept;

    /**
     * @brief Load a plugin from a shared library.
     *
     * The library must export YOUNITY_PLUGIN_DESCRIPTOR and the descriptor must match the
     * ABI of the engine. The context is created before the call returns.
     *
     * @param path The path of the shared library, with its platform extension
     *
     * @return The loaded plugin, or the reason of the failure.
     */
    [[nodiscard]] static std::expected<Plugin, PluginError> loadFromFile(const std::filesystem::path &path);

    /**
     * @brief Get the name the plugin gives in its descriptor.
     *
     * @return The name, or an empty view once the plugin is unloaded.
     */
    [[nodiscard]] std::string_view getName(void) const noexcept;

    /**
     * @brief Get the version the plugin gives in its descriptor.
     *
     * @return The version, or an empty view once the plugin is unloaded.
     */
    [[nodiscard]] std::string_view getVersion(void) const noexcept;

    /**
     * @brief Get the path the plugin was loaded from.
     *
     * @return The path, or an empty path once the plugin is unloaded.
     */
    [[nodiscard]] const std::filesystem::path &getPath(void) const noexcept;

    /**
     * @brief Check if the plugin is between start() and stop().
     *
     * @return true once start() returned, until stop() or unload() is called.
     */
    [[nodiscard]] bool isStarted(void) const noexcept;

    /**
     * @brief Call the start callback of the plugin, once.
     */
    void start(void) noexcept;

    /**
     * @brief Call the update callback of the plugin while it is started.
     *
     * @param deltaTime The time elapsed since the last update, in seconds
     */
    void update(double deltaTime) noexcept;

    /**
     * @brief Call the stop callback of the plugin if it is started.
     */
    void stop(void) noexcept;

    /**
     * @brief Stop the plugin, destroy its context and close the library.
     */
    void unload(void) noexcept;

private:
    Plugin(
        void *library,
        const PluginDescriptor *descriptor,
        void *instance,
        std::filesystem::path path,
        std::string name,
        std::string version) noexcept;

    void *_library = nullptr;
    const PluginDescriptor *_descriptor = nullptr;
    void *_instance = nullptr;
    std::filesystem::path _path;
    std::string _name;
    std::string _version;
    bool _started = false;
};

} // namespace younity::plugin

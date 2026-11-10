/*
** EPITECH PROJECT, 2026
** younity-engine/modules/plugin/include/younity/plugin/PluginManager
** File description:
** PluginManager header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <vector>

#include "younity/plugin/Plugin.hpp"
#include "younity/plugin/PluginError.hpp"

namespace younity::plugin {

/**
 * @brief The plugins of a game, loaded from a JSON configuration and driven by the game loop.
 *
 * loadFromFile reads the configuration, loads every plugin it lists and replaces the
 * current ones; a failure leaves the current plugins untouched. The reload stops the
 * plugins already started, and the new ones are started by the next start().
 *
 * A configuration is an object with one key, "plugins", holding the paths of the shared
 * libraries. A relative path is resolved from the folder of the configuration file:
 *
 *     {
 *         "plugins": [
 *             "plugins/weapons.so"
 *         ]
 *     }
 */
class PluginManager
{
public:
    /**
     * @brief Create a manager without any plugin.
     */
    PluginManager(void) = default;

    /**
     * @brief Stop and unload every plugin.
     */
    ~PluginManager(void);

    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;
    PluginManager(PluginManager &&) = delete;
    PluginManager &operator=(PluginManager &&) = delete;

    /**
     * @brief Replace the plugins with the ones listed in a JSON configuration.
     *
     * The new plugins are loaded into a temporary list first, so a failure leaves the
     * current plugins, and their started state, untouched. On success, the current plugins
     * are stopped and unloaded, then replaced; the new ones are not started.
     *
     * @param configPath The path of the configuration file
     *
     * @return Nothing on success, or the reason of the failure.
     */
    [[nodiscard]] std::expected<void, PluginError> loadFromFile(const std::filesystem::path &configPath);

    /**
     * @brief Start every plugin in the order of the configuration, once.
     */
    void start(void) noexcept;

    /**
     * @brief Call update on every plugin while the manager is started.
     *
     * @param deltaTime The time elapsed since the last update, in seconds
     */
    void update(double deltaTime) noexcept;

    /**
     * @brief Stop every started plugin in the reverse order of the configuration.
     */
    void stop(void) noexcept;

    /**
     * @brief Stop every plugin and unload them.
     */
    void clear(void) noexcept;

    /**
     * @brief Check if the plugins are started.
     *
     * @return true once start() returned, until stop() or clear() is called.
     */
    [[nodiscard]] bool isStarted(void) const noexcept;

    /**
     * @brief Get the loaded plugins, in the order of the configuration.
     *
     * @return The plugins, empty when none is loaded.
     */
    [[nodiscard]] const std::vector<Plugin> &getPlugins(void) const noexcept;

private:
    std::vector<Plugin> _plugins;
    bool _started = false;
};

} // namespace younity::plugin

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/plugin/src/PluginManager
** File description:
** PluginManager source file
*/

#include "younity/plugin/PluginManager.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

namespace younity::plugin {

namespace {

using Json = nlohmann::json;

[[nodiscard]] PluginError makeError(
    PluginErrorCode code,
    const std::filesystem::path &path,
    std::size_t line,
    std::string message)
{
    return PluginError{.code = code, .path = path, .line = line, .message = std::move(message)};
}

[[nodiscard]] std::size_t lineOf(const std::string &content, std::size_t byte)
{
    if (byte == 0) {
        return 0;
    }
    const std::size_t end = std::min(byte, content.size());
    const std::string_view prefix(content.data(), end);

    return 1 + static_cast<std::size_t>(std::count(prefix.begin(), prefix.end(), '\n'));
}

[[nodiscard]] std::expected<std::vector<std::filesystem::path>, PluginError> readPluginPaths(
    const std::filesystem::path &configPath)
{
    std::error_code fileError;

    if (configPath.empty() || !std::filesystem::exists(configPath, fileError)) {
        if (fileError) {
            return std::unexpected(makeError(
                PluginErrorCode::OpenFailed, configPath, 0,
                "cannot check the plugin configuration: " + fileError.message()));
        }
        return std::unexpected(makeError(
            PluginErrorCode::NotFound, configPath, 0, "the plugin configuration does not exist"));
    }

    std::ifstream input(configPath, std::ios::binary);

    if (!input.is_open()) {
        return std::unexpected(makeError(
            PluginErrorCode::OpenFailed, configPath, 0, "cannot open the plugin configuration for reading"));
    }
    std::string content;

    try {
        content.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    } catch (const std::exception &error) {
        // Reading a directory opens the stream and throws on the first underflow
        return std::unexpected(makeError(
            PluginErrorCode::OpenFailed, configPath, 0,
            "cannot read the plugin configuration: " + std::string(error.what())));
    }
    if (input.bad()) {
        return std::unexpected(
            makeError(PluginErrorCode::OpenFailed, configPath, 0, "cannot read the plugin configuration"));
    }

    Json root;

    try {
        root = Json::parse(content);
    } catch (const Json::parse_error &error) {
        return std::unexpected(makeError(
            PluginErrorCode::InvalidConfig, configPath, lineOf(content, error.byte),
            "invalid JSON: " + std::string(error.what())));
    } catch (const std::exception &error) {
        return std::unexpected(makeError(
            PluginErrorCode::InvalidConfig, configPath, 0, "invalid JSON: " + std::string(error.what())));
    }
    if (!root.is_object()) {
        return std::unexpected(
            makeError(PluginErrorCode::InvalidConfig, configPath, 0, "the configuration must be a JSON object"));
    }
    for (const auto &item : root.items()) {
        if (item.key() != "plugins") {
            return std::unexpected(makeError(
                PluginErrorCode::InvalidConfig, configPath, 0, "unknown key \"" + item.key() + "\""));
        }
    }
    const auto plugins = root.find("plugins");

    if (plugins == root.end()) {
        return std::unexpected(
            makeError(PluginErrorCode::InvalidConfig, configPath, 0, "the configuration has no \"plugins\""));
    }
    if (!plugins->is_array()) {
        return std::unexpected(makeError(
            PluginErrorCode::InvalidConfig, configPath, 0, "the \"plugins\" value must be an array of paths"));
    }

    std::vector<std::filesystem::path> paths;

    paths.reserve(plugins->size());
    for (const Json &entry : *plugins) {
        if (!entry.is_string()) {
            return std::unexpected(makeError(
                PluginErrorCode::InvalidConfig, configPath, 0, "every plugin path must be a string"));
        }
        const std::string value = entry.get<std::string>();

        if (value.empty()) {
            return std::unexpected(
                makeError(PluginErrorCode::InvalidConfig, configPath, 0, "a plugin path is empty"));
        }
        std::filesystem::path pluginPath(value);

        if (pluginPath.is_relative()) {
            pluginPath = configPath.parent_path() / pluginPath;
        }
        paths.push_back(std::move(pluginPath));
    }
    return paths;
}

} // namespace

PluginManager::~PluginManager(void)
{
    this->clear();
}

std::expected<void, PluginError> PluginManager::loadFromFile(const std::filesystem::path &configPath)
{
    std::expected<std::vector<std::filesystem::path>, PluginError> paths = readPluginPaths(configPath);

    if (!paths) {
        return std::unexpected(std::move(paths.error()));
    }

    std::vector<Plugin> loaded;

    loaded.reserve(paths->size());
    for (const std::filesystem::path &path : *paths) {
        std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(path);

        if (!plugin) {
            return std::unexpected(std::move(plugin.error()));
        }
        loaded.push_back(std::move(*plugin));
    }
    this->stop();
    this->_plugins = std::move(loaded);
    return {};
}

void PluginManager::start(void) noexcept
{
    if (this->_started) {
        return;
    }
    for (Plugin &plugin : this->_plugins) {
        plugin.start();
    }
    this->_started = true;
}

void PluginManager::update(double deltaTime) noexcept
{
    if (!this->_started) {
        return;
    }
    for (Plugin &plugin : this->_plugins) {
        plugin.update(deltaTime);
    }
}

void PluginManager::stop(void) noexcept
{
    if (!this->_started) {
        return;
    }
    for (auto it = this->_plugins.rbegin(); it != this->_plugins.rend(); ++it) {
        it->stop();
    }
    this->_started = false;
}

void PluginManager::clear(void) noexcept
{
    this->stop();
    this->_plugins.clear();
}

bool PluginManager::isStarted(void) const noexcept
{
    return this->_started;
}

const std::vector<Plugin> &PluginManager::getPlugins(void) const noexcept
{
    return this->_plugins;
}

} // namespace younity::plugin

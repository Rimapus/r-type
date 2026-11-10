/*
** EPITECH PROJECT, 2026
** younity-engine/tests/plugin/PluginManager
** File description:
** PluginManager unit tests
*/

#include <criterion/criterion.h>

#include <array>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "younity/core/Version.hpp"
#include "younity/plugin/PluginError.hpp"
#include "younity/plugin/PluginManager.hpp"

using younity::plugin::PluginError;
using younity::plugin::PluginErrorCode;
using younity::plugin::PluginManager;

namespace {

constexpr const char *LOG_VARIABLE = "YOUNITY_TEST_PLUGIN_LOG";

class TempConfig
{
public:
    explicit TempConfig(const char *name)
        : _path(std::filesystem::temp_directory_path() / ("younity_plugin_config_" + std::string(name) + ".json"))
    {
    }

    ~TempConfig(void)
    {
        std::error_code error;

        std::filesystem::remove(this->_path, error);
    }

    TempConfig(const TempConfig &) = delete;
    TempConfig &operator=(const TempConfig &) = delete;

    [[nodiscard]] const std::filesystem::path &getPath(void) const noexcept
    {
        return this->_path;
    }

    void write(std::string_view content) const
    {
        std::ofstream output(this->_path, std::ios::binary | std::ios::trunc);

        output.write(content.data(), static_cast<std::streamsize>(content.size()));
        output.close();
        if (!output) {
            throw std::runtime_error("cannot write the test plugin configuration");
        }
    }

private:
    std::filesystem::path _path;
};

std::filesystem::path logPath(const char *test)
{
    return std::filesystem::temp_directory_path() / ("younity_plugin_" + std::string(test) + ".log");
}

void prepareLog(const std::filesystem::path &path)
{
    std::error_code error;

    std::filesystem::remove(path, error);
    setenv(LOG_VARIABLE, path.c_str(), 1);
}

std::string readLog(const std::filesystem::path &path)
{
    std::ifstream input(path);
    std::ostringstream content;

    content << input.rdbuf();
    return content.str();
}

void cleanLog(const std::filesystem::path &path)
{
    unsetenv(LOG_VARIABLE);
    std::error_code error;

    std::filesystem::remove(path, error);
}

std::string jsonPath(const std::filesystem::path &path)
{
    const std::string value = path.generic_string();
    std::string escaped;

    escaped.reserve(value.size() + 2);
    escaped += '"';
    for (const char character : value) {
        if (character == '\\' || character == '"') {
            escaped += '\\';
        }
        escaped += character;
    }
    escaped += '"';
    return escaped;
}

std::string configOf(const std::vector<std::filesystem::path> &paths)
{
    std::string config = "{\"plugins\":[";

    for (std::size_t index = 0; index < paths.size(); ++index) {
        if (index > 0) {
            config += ',';
        }
        config += jsonPath(paths[index]);
    }
    config += "]}";
    return config;
}

std::string nameOf(const PluginManager &manager, std::size_t index)
{
    return std::string(manager.getPlugins()[index].getName());
}

} // namespace

Test(plugin_manager, loads_the_plugins_of_a_configuration)
{
    TempConfig config("loads");

    config.write(configOf({YOUNITY_TEST_PLUGIN_VALID}));
    PluginManager manager;
    const std::expected<void, PluginError> loaded = manager.loadFromFile(config.getPath());

    cr_assert(loaded.has_value());
    cr_assert_eq(manager.getPlugins().size(), std::size_t{1});
    const std::string name = nameOf(manager, 0);

    cr_assert_str_eq(name.c_str(), "test-plugin");
    cr_assert_not(manager.isStarted());
}

Test(plugin_manager, resolves_relative_paths_from_the_configuration)
{
    TempConfig config("relative");

    config.write(configOf({std::filesystem::relative(YOUNITY_TEST_PLUGIN_VALID, config.getPath().parent_path())}));
    PluginManager manager;
    const std::expected<void, PluginError> loaded = manager.loadFromFile(config.getPath());

    cr_assert(loaded.has_value());
    cr_assert_eq(manager.getPlugins().size(), std::size_t{1});
    const std::string name = nameOf(manager, 0);

    cr_assert_str_eq(name.c_str(), "test-plugin");
}

Test(plugin_manager, loads_an_empty_configuration)
{
    TempConfig config("empty");

    config.write("{\"plugins\": []}");
    PluginManager manager;
    const std::expected<void, PluginError> loaded = manager.loadFromFile(config.getPath());

    cr_assert(loaded.has_value());
    cr_assert(manager.getPlugins().empty());
    cr_assert_not(manager.isStarted());
}

Test(plugin_manager, starts_updates_and_stops_in_order)
{
    TempConfig config("order");
    const std::filesystem::path log = logPath("manager_order");

    config.write(configOf({YOUNITY_TEST_PLUGIN_FIRST, YOUNITY_TEST_PLUGIN_SECOND}));
    prepareLog(log);
    {
        PluginManager manager;

        cr_assert(manager.loadFromFile(config.getPath()).has_value());
        manager.start();
        cr_assert(manager.isStarted());
        manager.update(0.25);
        manager.stop();
        cr_assert_not(manager.isStarted());
        unsetenv(LOG_VARIABLE);
        const std::string expected = std::format(
            "create first\ncreate second\n"
            "start first {}\nstart second {}\n"
            "update first 0.250000\nupdate second 0.250000\n"
            "stop second\nstop first\n",
            younity::version(), younity::version());
        const std::string logContent = readLog(log);

        cr_assert_str_eq(logContent.c_str(), expected.c_str());
    }
    cleanLog(log);
}

Test(plugin_manager, does_nothing_before_start)
{
    TempConfig config("before_start");
    const std::filesystem::path log = logPath("manager_before_start");

    config.write(configOf({YOUNITY_TEST_PLUGIN_VALID}));
    prepareLog(log);
    {
        PluginManager manager;

        cr_assert(manager.loadFromFile(config.getPath()).has_value());
        manager.update(0.5);
        cr_assert_not(manager.isStarted());
        unsetenv(LOG_VARIABLE);
        const std::string logContent = readLog(log);

        cr_assert_str_eq(logContent.c_str(), "create test-plugin\n");
    }
    cleanLog(log);
}

Test(plugin_manager, start_and_stop_are_idempotent)
{
    TempConfig config("idempotent");
    const std::filesystem::path log = logPath("manager_idempotent");

    config.write(configOf({YOUNITY_TEST_PLUGIN_VALID}));
    prepareLog(log);
    {
        PluginManager manager;

        cr_assert(manager.loadFromFile(config.getPath()).has_value());
        manager.start();
        manager.start();
        manager.stop();
        manager.stop();
        cr_assert_not(manager.isStarted());
        unsetenv(LOG_VARIABLE);
        const std::string expected =
            std::format("create test-plugin\nstart test-plugin {}\nstop test-plugin\n", younity::version());
        const std::string logContent = readLog(log);

        cr_assert_str_eq(logContent.c_str(), expected.c_str());
    }
    cleanLog(log);
}

Test(plugin_manager, clear_stops_and_unloads_the_plugins)
{
    TempConfig config("clear");
    const std::filesystem::path log = logPath("manager_clear");

    config.write(configOf({YOUNITY_TEST_PLUGIN_VALID}));
    prepareLog(log);
    PluginManager manager;

    cr_assert(manager.loadFromFile(config.getPath()).has_value());
    manager.start();
    manager.clear();
    cr_assert(manager.getPlugins().empty());
    cr_assert_not(manager.isStarted());
    unsetenv(LOG_VARIABLE);
    const std::string expected = std::format(
        "create test-plugin\nstart test-plugin {}\nstop test-plugin\ndestroy test-plugin\n", younity::version());
    const std::string logContent = readLog(log);

    cr_assert_str_eq(logContent.c_str(), expected.c_str());
    cleanLog(log);
}

Test(plugin_manager, does_not_change_the_plugins_when_a_configuration_fails)
{
    TempConfig good("atomic_good");
    TempConfig bad("atomic_bad");
    const std::filesystem::path missing =
        std::filesystem::temp_directory_path() / "younity_plugin_missing_from_config.so";
    std::error_code error;

    std::filesystem::remove(missing, error);
    good.write(configOf({YOUNITY_TEST_PLUGIN_VALID}));
    bad.write(configOf({YOUNITY_TEST_PLUGIN_VALID, missing}));
    PluginManager manager;

    cr_assert(manager.loadFromFile(good.getPath()).has_value());
    manager.start();
    const std::expected<void, PluginError> loaded = manager.loadFromFile(bad.getPath());

    cr_assert_not(loaded.has_value());
    cr_assert_eq(loaded.error().code, PluginErrorCode::NotFound);
    cr_assert_eq(manager.getPlugins().size(), std::size_t{1});
    const std::string name = nameOf(manager, 0);

    cr_assert_str_eq(name.c_str(), "test-plugin");
    cr_assert(manager.isStarted());
}

Test(plugin_manager, stops_the_previous_plugins_when_a_configuration_loads)
{
    TempConfig first("replace_first");
    TempConfig second("replace_second");
    const std::filesystem::path log = logPath("manager_replace");

    first.write(configOf({YOUNITY_TEST_PLUGIN_VALID}));
    second.write(configOf({YOUNITY_TEST_PLUGIN_FIRST}));
    prepareLog(log);
    {
        PluginManager manager;

        cr_assert(manager.loadFromFile(first.getPath()).has_value());
        manager.start();
        cr_assert(manager.loadFromFile(second.getPath()).has_value());
        cr_assert_not(manager.isStarted());
        cr_assert_eq(manager.getPlugins().size(), std::size_t{1});
        const std::string name = nameOf(manager, 0);

        cr_assert_str_eq(name.c_str(), "first");
        unsetenv(LOG_VARIABLE);
        const std::string expected = std::format(
            "create test-plugin\nstart test-plugin {}\ncreate first\nstop test-plugin\ndestroy test-plugin\n",
            younity::version());
        const std::string logContent = readLog(log);

        cr_assert_str_eq(logContent.c_str(), expected.c_str());
    }
    cleanLog(log);
}

Test(plugin_manager, reports_a_missing_configuration)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "younity_plugin_config_does_not_exist.json";
    std::error_code error;

    std::filesystem::remove(path, error);
    PluginManager manager;
    const std::expected<void, PluginError> loaded = manager.loadFromFile(path);

    cr_assert_not(loaded.has_value());
    cr_assert_eq(loaded.error().code, PluginErrorCode::NotFound);
    cr_assert_eq(loaded.error().path.string(), path.string());
}

Test(plugin_manager, reports_a_configuration_that_cannot_be_read)
{
    PluginManager manager;
    const std::expected<void, PluginError> loaded = manager.loadFromFile(std::filesystem::temp_directory_path());

    cr_assert_not(loaded.has_value());
    cr_assert_eq(loaded.error().code, PluginErrorCode::OpenFailed);
}

Test(plugin_manager, reports_a_syntax_error_with_its_line)
{
    TempConfig config("syntax");

    config.write("{\n  \"plugins\": [\n}\n");
    PluginManager manager;
    const std::expected<void, PluginError> loaded = manager.loadFromFile(config.getPath());

    cr_assert_not(loaded.has_value());
    cr_assert_eq(loaded.error().code, PluginErrorCode::InvalidConfig);
    cr_assert(loaded.error().line > 0);
}

Test(plugin_manager, reports_an_invalid_configuration)
{
    struct Case
    {
        const char *content;
        const char *message;
    };

    const std::array<Case, 6> cases{
        {
         {"[]", "the configuration must be a JSON object"},
         {"{}", "the configuration has no \"plugins\""},
         {"{\"plugins\": 1}", "the \"plugins\" value must be an array of paths"},
         {"{\"plugins\": [1]}", "every plugin path must be a string"},
         {"{\"plugins\": [\"\"]}", "a plugin path is empty"},
         {"{\"plugins\": [], \"other\": 1}", "unknown key \"other\""},
         }
    };
    TempConfig config("invalid");

    for (const Case &testCase : cases) {
        config.write(testCase.content);
        PluginManager manager;
        const std::expected<void, PluginError> loaded = manager.loadFromFile(config.getPath());

        cr_assert_not(loaded.has_value());
        cr_assert_eq(loaded.error().code, PluginErrorCode::InvalidConfig);
        cr_assert_str_eq(loaded.error().message.c_str(), testCase.message);
    }
}

Test(plugin_manager, reports_a_plugin_error_from_the_configuration)
{
    TempConfig config("plugin_error");
    const std::filesystem::path missing =
        std::filesystem::temp_directory_path() / "younity_plugin_missing_from_error_config.so";
    std::error_code error;

    std::filesystem::remove(missing, error);
    config.write(configOf({missing}));
    PluginManager manager;
    const std::expected<void, PluginError> loaded = manager.loadFromFile(config.getPath());

    cr_assert_not(loaded.has_value());
    cr_assert_eq(loaded.error().code, PluginErrorCode::NotFound);
    cr_assert_eq(loaded.error().path.string(), missing.string());
}

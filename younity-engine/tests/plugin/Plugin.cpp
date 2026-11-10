/*
** EPITECH PROJECT, 2026
** younity-engine/tests/plugin/Plugin
** File description:
** Plugin unit tests
*/

#include <criterion/criterion.h>

#include <array>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

#include "younity/core/Version.hpp"
#include "younity/plugin/Plugin.hpp"
#include "younity/plugin/PluginError.hpp"

using younity::plugin::Plugin;
using younity::plugin::PluginError;
using younity::plugin::PluginErrorCode;

namespace {

constexpr const char *LOG_VARIABLE = "YOUNITY_TEST_PLUGIN_LOG";

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

std::string nameOf(const Plugin &plugin)
{
    return std::string(plugin.getName());
}

} // namespace

Test(plugin, loads_a_plugin_and_exposes_its_metadata)
{
    const std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_VALID);

    cr_assert(plugin.has_value());
    const std::string name = nameOf(*plugin);
    const std::string version = std::string(plugin->getVersion());

    cr_assert_str_eq(name.c_str(), "test-plugin");
    cr_assert_str_eq(version.c_str(), "1.0.0");
    cr_assert_eq(plugin->getPath().string(), std::string(YOUNITY_TEST_PLUGIN_VALID));
    cr_assert_not(plugin->isStarted());
}

Test(plugin, calls_the_lifecycle_in_order)
{
    const std::filesystem::path path = logPath("lifecycle");

    prepareLog(path);
    {
        std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_VALID);

        cr_assert(plugin.has_value());
        plugin->start();
        cr_assert(plugin->isStarted());
        plugin->update(0.5);
        plugin->stop();
        cr_assert_not(plugin->isStarted());
    }
    unsetenv(LOG_VARIABLE);
    const std::string expected = std::format(
        "create test-plugin\nstart test-plugin {}\nupdate test-plugin 0.500000\nstop test-plugin\ndestroy test-plugin\n",
        younity::version());
    const std::string log = readLog(path);

    cr_assert_str_eq(log.c_str(), expected.c_str());
    cleanLog(path);
}

Test(plugin, start_and_stop_are_idempotent)
{
    const std::filesystem::path path = logPath("idempotent");

    prepareLog(path);
    {
        std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_VALID);

        cr_assert(plugin.has_value());
        plugin->start();
        plugin->start();
        plugin->update(1.0);
        plugin->stop();
        plugin->stop();
    }
    unsetenv(LOG_VARIABLE);
    const std::string expected = std::format(
        "create test-plugin\nstart test-plugin {}\nupdate test-plugin 1.000000\nstop test-plugin\ndestroy test-plugin\n",
        younity::version());
    const std::string log = readLog(path);

    cr_assert_str_eq(log.c_str(), expected.c_str());
    cleanLog(path);
}

Test(plugin, does_nothing_before_start)
{
    const std::filesystem::path path = logPath("before_start");

    prepareLog(path);
    {
        std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_VALID);

        cr_assert(plugin.has_value());
        plugin->update(0.5);
        plugin->stop();
    }
    unsetenv(LOG_VARIABLE);
    const std::string log = readLog(path);

    cr_assert_str_eq(log.c_str(), "create test-plugin\ndestroy test-plugin\n");
    cleanLog(path);
}

Test(plugin, moves_keep_the_plugin_loaded)
{
    const std::filesystem::path path = logPath("move");

    prepareLog(path);
    {
        std::expected<Plugin, PluginError> first = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_VALID);

        cr_assert(first.has_value());
        Plugin second = std::move(*first);

        cr_assert(first->getName().empty());
        cr_assert(first->getPath().empty());
        const std::string name = nameOf(second);

        cr_assert_str_eq(name.c_str(), "test-plugin");
        second.start();
        cr_assert(second.isStarted());
    }
    unsetenv(LOG_VARIABLE);
    const std::string expected = std::format(
        "create test-plugin\nstart test-plugin {}\nstop test-plugin\ndestroy test-plugin\n", younity::version());
    const std::string log = readLog(path);

    cr_assert_str_eq(log.c_str(), expected.c_str());
    cleanLog(path);
}

Test(plugin, move_assignment_unloads_the_previous_plugin)
{
    const std::filesystem::path path = logPath("move_assignment");

    prepareLog(path);
    {
        std::expected<Plugin, PluginError> first = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_VALID);
        std::expected<Plugin, PluginError> second = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_VALID);

        cr_assert(first.has_value());
        cr_assert(second.has_value());
        *second = std::move(*first);
        const std::string name = nameOf(*second);

        cr_assert_str_eq(name.c_str(), "test-plugin");
    }
    unsetenv(LOG_VARIABLE);
    const std::string log = readLog(path);

    cr_assert_str_eq(log.c_str(), "create test-plugin\ncreate test-plugin\ndestroy test-plugin\ndestroy test-plugin\n");
    cleanLog(path);
}

Test(plugin, destroys_a_plugin_without_a_context)
{
    const std::filesystem::path path = logPath("stateless");

    prepareLog(path);
    {
        std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_STATELESS);

        cr_assert(plugin.has_value());
        plugin->start();
        plugin->stop();
    }
    unsetenv(LOG_VARIABLE);
    const std::string log = readLog(path);

    cr_assert_str_eq(log.c_str(), "start stateless\nstop stateless\ndestroy stateless\n");
    cleanLog(path);
}

Test(plugin, reports_a_missing_file)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "younity_plugin_does_not_exist.so";
    std::error_code error;

    std::filesystem::remove(path, error);
    const std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(path);

    cr_assert_not(plugin.has_value());
    cr_assert_eq(plugin.error().code, PluginErrorCode::NotFound);
    cr_assert_eq(plugin.error().path.string(), path.string());
}

Test(plugin, reports_a_file_that_is_not_a_library)
{
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "younity_plugin_not_a_library.so";

    {
        std::ofstream output(path);

        output << "not a shared library";
    }
    const std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(path);

    cr_assert_not(plugin.has_value());
    cr_assert_eq(plugin.error().code, PluginErrorCode::OpenFailed);
    std::error_code error;

    std::filesystem::remove(path, error);
}

Test(plugin, reports_a_library_without_a_descriptor)
{
    const std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_MISSING_DESCRIPTOR);

    cr_assert_not(plugin.has_value());
    cr_assert_eq(plugin.error().code, PluginErrorCode::MissingDescriptor);
    cr_assert_str_eq(plugin.error().message.c_str(), "the plugin does not export younity_plugin_descriptor");
}

Test(plugin, reports_an_abi_mismatch)
{
    const std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_BAD_ABI);

    cr_assert_not(plugin.has_value());
    cr_assert_eq(plugin.error().code, PluginErrorCode::AbiMismatch);
    const std::string expected = std::format(
        "the plugin ABI version is {}, the engine expects {}", YOUNITY_PLUGIN_ABI_VERSION + 1,
        YOUNITY_PLUGIN_ABI_VERSION);

    cr_assert_str_eq(plugin.error().message.c_str(), expected.c_str());
}

Test(plugin, reports_a_null_descriptor)
{
    const std::expected<Plugin, PluginError> plugin = Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_NULL_DESCRIPTOR);

    cr_assert_not(plugin.has_value());
    cr_assert_eq(plugin.error().code, PluginErrorCode::InvalidDescriptor);
    cr_assert_str_eq(plugin.error().message.c_str(), "the plugin descriptor is null");
}

Test(plugin, reports_an_invalid_descriptor)
{
    struct Case
    {
        const char *field;
        const char *message;
    };

    const std::array<Case, 9> cases{
        {
         {"name", "the plugin descriptor has no name"},
         {"empty-name", "the plugin descriptor has no name"},
         {"version", "the plugin descriptor has no version"},
         {"empty-version", "the plugin descriptor has no version"},
         {"create", "the plugin descriptor has no create callback"},
         {"destroy", "the plugin descriptor has no destroy callback"},
         {"start", "the plugin descriptor has no start callback"},
         {"update", "the plugin descriptor has no update callback"},
         {"stop", "the plugin descriptor has no stop callback"},
         }
    };

    for (const Case &testCase : cases) {
        setenv("YOUNITY_TEST_PLUGIN_INVALID", testCase.field, 1);
        const std::expected<Plugin, PluginError> plugin =
            Plugin::loadFromFile(YOUNITY_TEST_PLUGIN_INVALID_DESCRIPTOR);

        cr_assert_not(plugin.has_value());
        cr_assert_eq(plugin.error().code, PluginErrorCode::InvalidDescriptor);
        cr_assert_str_eq(plugin.error().message.c_str(), testCase.message);
    }
    unsetenv("YOUNITY_TEST_PLUGIN_INVALID");
}

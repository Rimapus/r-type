/*
** EPITECH PROJECT, 2026
** younity-engine/tests/plugin/fixtures/valid
** File description:
** Valid plugin fixture
*/

#include <cstdlib>
#include <format>
#include <fstream>
#include <string>
#include <string_view>

#include "younity/core/Version.hpp"
#include "younity/plugin/PluginAPI.hpp"

#ifndef YOUNITY_TEST_PLUGIN_NAME
#define YOUNITY_TEST_PLUGIN_NAME "test-plugin"
#endif

#ifndef YOUNITY_TEST_PLUGIN_VERSION
#define YOUNITY_TEST_PLUGIN_VERSION "1.0.0"
#endif

namespace {

constexpr const char *LOG_VARIABLE = "YOUNITY_TEST_PLUGIN_LOG";

void logEvent(std::string_view event)
{
    const char *const path = std::getenv(LOG_VARIABLE);

    if (path == nullptr) {
        return;
    }
    std::ofstream file(path, std::ios::app);

    file << event << '\n';
}

void *create(void)
{
    logEvent(std::format("create {}", YOUNITY_TEST_PLUGIN_NAME));
    return new int(0);
}

void destroy(void *context)
{
    logEvent(std::format("destroy {}", YOUNITY_TEST_PLUGIN_NAME));
    delete static_cast<int *>(context);
}

void start(void * /* context */)
{
    logEvent(std::format("start {} {}", YOUNITY_TEST_PLUGIN_NAME, younity::version()));
}

void update(void * /* context */, double deltaTime)
{
    logEvent(std::format("update {} {:.6f}", YOUNITY_TEST_PLUGIN_NAME, deltaTime));
}

void stop(void * /* context */)
{
    logEvent(std::format("stop {}", YOUNITY_TEST_PLUGIN_NAME));
}

const younity::plugin::PluginDescriptor DESCRIPTOR{
    .abiVersion = YOUNITY_PLUGIN_ABI_VERSION,
    .name = YOUNITY_TEST_PLUGIN_NAME,
    .version = YOUNITY_TEST_PLUGIN_VERSION,
    .create = create,
    .destroy = destroy,
    .start = start,
    .update = update,
    .stop = stop,
};

} // namespace

YOUNITY_PLUGIN_DESCRIPTOR
{
    return &DESCRIPTOR;
}

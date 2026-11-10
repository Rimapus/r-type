/*
** EPITECH PROJECT, 2026
** younity-engine/tests/plugin/fixtures/stateless
** File description:
** Plugin fixture without a context
*/

#include <cstdlib>
#include <fstream>
#include <string_view>

#include "younity/plugin/PluginAPI.hpp"

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
    return nullptr;
}

void destroy(void * /* context */)
{
    logEvent("destroy stateless");
}

void start(void * /* context */)
{
    logEvent("start stateless");
}

void update(void * /* context */, double deltaTime)
{
    (void)deltaTime;
}

void stop(void * /* context */)
{
    logEvent("stop stateless");
}

const younity::plugin::PluginDescriptor DESCRIPTOR{
    .abiVersion = YOUNITY_PLUGIN_ABI_VERSION,
    .name = "stateless",
    .version = "1.0.0",
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

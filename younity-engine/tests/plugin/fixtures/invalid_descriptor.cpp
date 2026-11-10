/*
** EPITECH PROJECT, 2026
** younity-engine/tests/plugin/fixtures/invalid_descriptor
** File description:
** Plugin fixture with one invalid descriptor field, chosen by an environment variable
*/

#include <cstdlib>
#include <string_view>

#include "younity/plugin/PluginAPI.hpp"

namespace {

void *create(void)
{
    return new int(0);
}

void destroy(void *context)
{
    delete static_cast<int *>(context);
}

void start(void * /* context */)
{
}

void update(void * /* context */, double /* deltaTime */)
{
}

void stop(void * /* context */)
{
}

younity::plugin::PluginDescriptor makeDescriptor(void)
{
    return younity::plugin::PluginDescriptor{
        .abiVersion = YOUNITY_PLUGIN_ABI_VERSION,
        .name = "invalid-plugin",
        .version = "1.0.0",
        .create = create,
        .destroy = destroy,
        .start = start,
        .update = update,
        .stop = stop,
    };
}

void breakField(younity::plugin::PluginDescriptor &descriptor, std::string_view field)
{
    if (field == "name") {
        descriptor.name = nullptr;
    } else if (field == "empty-name") {
        descriptor.name = "";
    } else if (field == "version") {
        descriptor.version = nullptr;
    } else if (field == "empty-version") {
        descriptor.version = "";
    } else if (field == "create") {
        descriptor.create = nullptr;
    } else if (field == "destroy") {
        descriptor.destroy = nullptr;
    } else if (field == "start") {
        descriptor.start = nullptr;
    } else if (field == "update") {
        descriptor.update = nullptr;
    } else if (field == "stop") {
        descriptor.stop = nullptr;
    }
}

} // namespace

YOUNITY_PLUGIN_DESCRIPTOR
{
    static younity::plugin::PluginDescriptor descriptor = makeDescriptor();
    const char *const field = std::getenv("YOUNITY_TEST_PLUGIN_INVALID");

    // Reset first: the same library may be loaded more than once in one process
    descriptor = makeDescriptor();
    if (field != nullptr) {
        breakField(descriptor, field);
    }
    return &descriptor;
}

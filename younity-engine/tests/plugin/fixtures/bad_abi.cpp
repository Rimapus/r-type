/*
** EPITECH PROJECT, 2026
** younity-engine/tests/plugin/fixtures/bad_abi
** File description:
** Plugin fixture built for another ABI version
*/

#include "younity/plugin/PluginAPI.hpp"

namespace {

void *create(void)
{
    return nullptr;
}

void destroy(void * /* context */)
{
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

const younity::plugin::PluginDescriptor DESCRIPTOR{
    .abiVersion = YOUNITY_PLUGIN_ABI_VERSION + 1,
    .name = "bad-abi",
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

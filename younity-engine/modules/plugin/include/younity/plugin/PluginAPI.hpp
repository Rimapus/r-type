/*
** EPITECH PROJECT, 2026
** younity-engine/modules/plugin/include/younity/plugin/PluginAPI
** File description:
** PluginAPI header file
*/

#pragma once

#include <cstdint>

/**
 * @brief Version of the plugin ABI, bumped whenever the descriptor or a callback changes.
 *
 * The engine refuses a plugin built for another version, so a changed layout is never
 * read with the wrong types.
 */
#define YOUNITY_PLUGIN_ABI_VERSION 1U

/**
 * @brief Name of the function a plugin exports to give its descriptor.
 *
 * Keep it in sync with YOUNITY_PLUGIN_DESCRIPTOR, which declares the function.
 */
#define YOUNITY_PLUGIN_DESCRIPTOR_SYMBOL "younity_plugin_descriptor"

#if defined(_WIN32)
/**
 * @brief Give a plugin function the C linkage and the visibility of a shared library export.
 */
#define YOUNITY_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
/**
 * @brief Give a plugin function the C linkage and the visibility of a shared library export.
 */
#define YOUNITY_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#else
/**
 * @brief Give a plugin function the C linkage and the visibility of a shared library export.
 */
#define YOUNITY_PLUGIN_EXPORT extern "C"
#endif

namespace younity::plugin {

/**
 * @brief What a plugin gives to the engine: its identity and the functions of its lifecycle.
 *
 * A plugin exports the function declared by YOUNITY_PLUGIN_DESCRIPTOR, which returns a
 * pointer to a descriptor with static storage. The engine copies the name and the version,
 * keeps the pointer for the lifetime of the plugin, and never writes to it.
 *
 * The context is the pointer returned by create. It is given to every other callback, and
 * may be null when the plugin has no state. The callbacks are C functions: they must not
 * throw, because the engine cannot catch an exception across the C ABI.
 */
struct PluginDescriptor
{
    /** @brief The ABI version the plugin was built for, YOUNITY_PLUGIN_ABI_VERSION. */
    std::uint32_t abiVersion = YOUNITY_PLUGIN_ABI_VERSION;
    /** @brief The name of the plugin, e.g. "weapons", never null or empty. */
    const char *name = nullptr;
    /** @brief The version of the plugin, never null or empty, e.g. "1.2.0". */
    const char *version = nullptr;
    /** @brief Create the context of the plugin, or return null when it has no state. */
    void *(*create)(void) = nullptr;
    /** @brief Destroy a context returned by create, called even when create returned null. */
    void (*destroy)(void *context) = nullptr;
    /** @brief Start the plugin, called once before its first update. */
    void (*start)(void *context) = nullptr;
    /** @brief Move the plugin forward, called once per update while it is started. */
    void (*update)(void *context, double deltaTime) = nullptr;
    /** @brief Stop the plugin, called once before it is destroyed or unloaded. */
    void (*stop)(void *context) = nullptr;
};

/**
 * @brief The type of the descriptor function a plugin exports.
 */
using PluginDescriptorFunction = const PluginDescriptor *(*)(void);

} // namespace younity::plugin

/**
 * @brief Declare the descriptor function of a plugin, with the symbol the engine looks for.
 *
 * Define it in one source of the plugin:
 *
 *     YOUNITY_PLUGIN_DESCRIPTOR
 *     {
 *         return &MY_DESCRIPTOR;
 *     }
 */
#define YOUNITY_PLUGIN_DESCRIPTOR \
    YOUNITY_PLUGIN_EXPORT const ::younity::plugin::PluginDescriptor *younity_plugin_descriptor(void)

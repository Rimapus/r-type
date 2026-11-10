/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/SceneFile
** File description:
** SceneFile header file
*/

#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <string>

#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"

namespace younity::ecs {

/**
 * @brief The largest id an entity of a scene file can have.
 *        A registry keeps a slot for each id up to the largest one, so the limit bounds the memory a file makes it take.
 */
inline constexpr Entity::Id MAX_SCENE_ENTITY_ID = (1U << 20U) - 1U;

/**
 * @brief Why a scene file could not be saved or loaded.
 */
struct SceneFileError
{
    /** @brief The file that was read or written. */
    std::filesystem::path path;
    /** @brief The line of a JSON syntax error, or 0 when the error has no line. */
    std::size_t line = 0;
    /** @brief The reason, with the JSON path of the invalid value when there is one. */
    std::string message;
};

/**
 * @brief Save the entities and components of a registry to a JSON scene file.
 *
 * Only the component types registered in the ComponentRegistry are written. The entities are sorted
 * by id and keep it, their components are sorted alphabetically and their fields in the order they were registered.
 *
 * @param registry The registry owning the entities and components
 * @param components The component types known by their name
 * @param path The file to write
 *
 * @return Nothing, or why the file could not be written, e.g. an entity id above MAX_SCENE_ENTITY_ID.
 */
[[nodiscard]] std::expected<void, SceneFileError> saveScene(
    const Registry &registry,
    const ComponentRegistry &components,
    const std::filesystem::path &path);

/**
 * @brief Replace the entities and components of a registry with the ones of a JSON scene file.
 *
 * The file is checked and built in a temporary registry first, so an invalid file leaves the
 * registry untouched. On success the entities have the ids of the file and the registry keeps its systems,
 * so an Entity kept by the game names the same entity after a save and a load.
 * Missing fields keep the default value of their component.
 *
 * @param registry The registry to replace, outside of updateSystems()
 * @param components The component types known by their name
 * @param path The file to read
 *
 * @return Nothing, or why the file could not be loaded.
 *
 * @throw std::logic_error if called during updateSystems(): the entities cannot be destroyed while a system runs.
 */
[[nodiscard]] std::expected<void, SceneFileError> loadScene(
    Registry &registry,
    const ComponentRegistry &components,
    const std::filesystem::path &path);

} // namespace younity::ecs

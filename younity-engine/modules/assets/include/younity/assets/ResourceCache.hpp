/*
** EPITECH PROJECT, 2026
** younity-engine/modules/assets/include/younity/assets/ResourceCache
** File description:
** ResourceCache header file
*/

#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "younity/assets/AssetError.hpp"
#include "younity/assets/AssetPath.hpp"
#include "younity/assets/Log.hpp"

namespace younity::assets {

/**
 * @brief Reads one resource from a file, e.g. a texture from a PNG.
 *
 * @tparam T The type of the resource
 */
template <typename T>
using Loader = std::function<std::expected<T, AssetError>(const std::filesystem::path &)>;

/**
 * @brief Loads the resources of one type and keeps them, so each file is read once.
 *
 * The files are found in the asset folder, and a path that goes out of it is refused.
 * The resources are shared: one kept by the game stays valid after it leaves the cache.
 *
 * @tparam T The type of the resource
 */
template <typename T>
class ResourceCache
{
public:
    /**
     * @brief Create an empty cache.
     *
     * @param loader The function that reads a resource from its file
     * @param root The asset folder, the current folder by default
     *
     * @throw std::invalid_argument if the loader is empty.
     */
    explicit ResourceCache(Loader<T> loader, std::filesystem::path root = {})
        : _loader(std::move(loader)), _root(std::move(root))
    {
        if (!this->_loader) {
            throw std::invalid_argument("younity::assets::ResourceCache: the loader is empty");
        }
    }

    /**
     * @brief Get the resource of a file, read by the loader the first time only.
     *
     * The loader receives the path inside the asset folder.
     * A failure is not kept: the next load of the same file calls the loader again.
     *
     * @param path The file of the resource, relative to the asset folder
     *
     * @return The resource, AssetError::InvalidPath if the path is refused,
     *         or why the loader could not read it.
     */
    [[nodiscard]] std::expected<std::shared_ptr<const T>, AssetError> load(const std::filesystem::path &path)
    {
        const std::expected<std::filesystem::path, AssetError> normal = normalizeAssetPath(path);

        if (!normal) {
            logger().warn("Cannot load {}: {}", path.generic_string(), toString(normal.error()));
            return std::unexpected(normal.error());
        }

        const std::string key = normal->generic_string();
        const auto cached = this->_resources.find(key);

        if (cached != this->_resources.end()) {
            logger().debug("{} comes from the cache", key);
            return cached->second;
        }

        std::expected<T, AssetError> loaded = this->_loader(this->_root / *normal);

        if (!loaded) {
            logger().warn("Cannot load {}: {}", key, toString(loaded.error()));
            return std::unexpected(loaded.error());
        }
        std::shared_ptr<const T> resource = std::make_shared<const T>(std::move(*loaded));

        this->_resources.emplace(key, resource);
        logger().info("Loaded {}", key);
        return resource;
    }

    /**
     * @brief Check if the resource of a file is in the cache.
     *
     * @param path The file of the resource, relative to the asset folder
     *
     * @return true if it has been loaded and not unloaded since.
     */
    [[nodiscard]] bool contains(const std::filesystem::path &path) const
    {
        const std::expected<std::filesystem::path, AssetError> normal = normalizeAssetPath(path);

        return normal && this->_resources.contains(normal->generic_string());
    }

    /**
     * @brief Find the file a resource of the cache was loaded from, e.g. to save it by its path.
     *        The resources are compared by address, one by one.
     *
     * @param resource The resource, as load() gave it
     *
     * @return Its path in the asset folder, or nothing for an empty pointer or a resource the cache does not hold.
     */
    [[nodiscard]] std::optional<std::filesystem::path> findPath(const std::shared_ptr<const T> &resource) const
    {
        if (resource == nullptr) {
            return std::nullopt;
        }
        for (const auto &[key, cached] : this->_resources) {
            if (cached == resource) {
                return std::filesystem::path(key);
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Remove the resource of a file from the cache, if it is there.
     *
     * @param path The file of the resource, relative to the asset folder
     */
    void unload(const std::filesystem::path &path)
    {
        const std::expected<std::filesystem::path, AssetError> normal = normalizeAssetPath(path);

        if (normal) {
            this->_resources.erase(normal->generic_string());
        }
    }

    /**
     * @brief Remove all the resources from the cache.
     */
    void clear(void) noexcept
    {
        this->_resources.clear();
    }

    /**
     * @brief Get the number of resources in the cache.
     *
     * @return The number of files loaded and not unloaded since.
     */
    [[nodiscard]] std::size_t getSize(void) const noexcept
    {
        return this->_resources.size();
    }

private:
    Loader<T> _loader;
    std::filesystem::path _root;
    // Keyed by the generic form of the normal path, the same on every platform
    std::unordered_map<std::string, std::shared_ptr<const T>> _resources;
};

} // namespace younity::assets

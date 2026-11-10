/*
** EPITECH PROJECT, 2026
** younity-engine/modules/assets/include/younity/assets/ResourceManager
** File description:
** ResourceManager header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

#include "younity/assets/AssetError.hpp"
#include "younity/assets/Log.hpp"
#include "younity/assets/ResourceCache.hpp"

namespace younity::assets {

/**
 * @brief Loads and keeps the resources of the game, whatever their type, from one asset folder.
 *
 * The game gives a loader for each type of resource, e.g. one for the textures and one for the sounds,
 * then asks for resources by type and path: load<Texture>("sprites/ship.png").
 * A loader must not call setLoader or clear for its own type: they destroy it while it runs.
 */
class ResourceManager
{
public:
    /**
     * @brief Create a manager without any loader.
     *
     * @param root The asset folder, the current folder by default
     */
    explicit ResourceManager(std::filesystem::path root = {})
        : _root(std::move(root)) {}

    /**
     * @brief Get the asset folder, the one the paths given to load() are relative to.
     *
     * @return The folder given to the constructor.
     */
    [[nodiscard]] const std::filesystem::path &getRoot(void) const noexcept
    {
        return this->_root;
    }

    /**
     * @brief Set how the resources of a type are read, replacing the previous loader of this type.
     *
     * The resources of this type loaded before are removed from the cache, so none comes from the old loader.
     *
     * @tparam T The type of the resource
     * @param loader The function that reads a resource from its file
     *
     * @throw std::invalid_argument if the loader is empty.
     */
    template <typename T>
    void setLoader(Loader<T> loader)
    {
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "a resource type is a plain type, not const nor a reference");
        this->_caches[std::type_index(typeid(T))] = std::make_unique<Cache<T>>(std::move(loader), this->_root);
    }

    /**
     * @brief Check if a type of resource has a loader.
     *
     * @tparam T The type of the resource
     *
     * @return true if setLoader has been called for this type.
     */
    template <typename T>
    [[nodiscard]] bool hasLoader(void) const
    {
        return this->_caches.contains(std::type_index(typeid(T)));
    }

    /**
     * @brief Get the resource of a file, read by the loader of its type the first time only.
     *
     * @tparam T The type of the resource
     * @param path The file of the resource, relative to the asset folder
     *
     * @return The resource, AssetError::NoLoader if its type has no loader,
     *         AssetError::InvalidPath if the path is refused, or why the loader could not read it.
     */
    template <typename T>
    [[nodiscard]] std::expected<std::shared_ptr<const T>, AssetError> load(const std::filesystem::path &path)
    {
        ResourceCache<T> *cache = this->findCache<T>();

        if (cache == nullptr) {
            logger().warn("Cannot load {}: {}", path.generic_string(), toString(AssetError::NoLoader));
            return std::unexpected(AssetError::NoLoader);
        }
        return cache->load(path);
    }

    /**
     * @brief Check if the resource of a file is in the cache of its type.
     *
     * @tparam T The type of the resource
     * @param path The file of the resource, relative to the asset folder
     *
     * @return true if it has been loaded and not unloaded since.
     */
    template <typename T>
    [[nodiscard]] bool contains(const std::filesystem::path &path) const
    {
        const ResourceCache<T> *cache = this->findCache<T>();

        return cache != nullptr && cache->contains(path);
    }

    /**
     * @brief Find the file a resource was loaded from, e.g. to save a sprite by the path of its texture.
     *
     * @tparam T The type of the resource
     * @param resource The resource, as load() gave it
     *
     * @return Its path in the asset folder, or nothing if its type has no loader or its cache does not hold it.
     */
    template <typename T>
    [[nodiscard]] std::optional<std::filesystem::path> findPath(const std::shared_ptr<const T> &resource) const
    {
        const ResourceCache<T> *cache = this->findCache<T>();

        if (cache == nullptr) {
            return std::nullopt;
        }
        return cache->findPath(resource);
    }

    /**
     * @brief Remove the resource of a file from the cache of its type, if it is there.
     *
     * @tparam T The type of the resource
     * @param path The file of the resource, relative to the asset folder
     */
    template <typename T>
    void unload(const std::filesystem::path &path)
    {
        ResourceCache<T> *cache = this->findCache<T>();

        if (cache != nullptr) {
            cache->unload(path);
        }
    }

    /**
     * @brief Remove the resources of every type from the cache, and keep the loaders.
     */
    void clear(void) noexcept
    {
        for (auto &[type, cache] : this->_caches) {
            cache->clear();
        }
    }

private:
    /**
     * @brief Base of the caches, whatever the type of their resources.
     */
    class ICache
    {
    public:
        virtual ~ICache(void) = default;

        virtual void clear(void) noexcept = 0;
    };

    /**
     * @brief The cache of one type of resource.
     *
     * @tparam T The type of the resource
     */
    template <typename T>
    struct Cache : public ICache
    {
        Cache(Loader<T> loader, const std::filesystem::path &root)
            : resources(std::move(loader), root) {}

        void clear(void) noexcept override
        {
            this->resources.clear();
        }

        ResourceCache<T> resources;
    };

    /**
     * @brief Get the cache of a type of resource.
     *
     * @tparam T The type of the resource
     *
     * @return The cache, or nullptr if the type has no loader.
     */
    template <typename T>
    [[nodiscard]] const ResourceCache<T> *findCache(void) const
    {
        // typeid ignores const and references: Foo and const Foo would share a cache of the wrong type
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "a resource type is a plain type, not const nor a reference");

        const auto found = this->_caches.find(std::type_index(typeid(T)));

        if (found == this->_caches.end()) {
            return nullptr;
        }
        return &static_cast<const Cache<T> &>(*found->second).resources;
    }

    /**
     * @brief Get the cache of a type of resource, to change it.
     *
     * @tparam T The type of the resource
     *
     * @return The cache, or nullptr if the type has no loader.
     */
    template <typename T>
    [[nodiscard]] ResourceCache<T> *findCache(void)
    {
        return const_cast<ResourceCache<T> *>(std::as_const(*this).template findCache<T>());
    }

    std::filesystem::path _root;
    std::unordered_map<std::type_index, std::unique_ptr<ICache>> _caches;
};

} // namespace younity::assets

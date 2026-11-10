/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/Registry
** File description:
** Registry header file
*/

#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Signature.hpp"
#include "younity/ecs/SparseArray.hpp"
#include "younity/ecs/System.hpp"

namespace younity::ecs {

/**
 * @brief Creates and destroys the entities, stores their components and runs the systems.
 */
class Registry
{
public:
    /**
     * @brief Called after a system runs, with the name of the system and the time it took in seconds.
     *
     * The registry has no dependency on the tools module: the game records the samples where it wants, e.g.
     * registry.setSystemProfiler([](std::string_view name, double seconds) {
     *     tools::metrics().record("system." + std::string(name), seconds, tools::MetricKind::Timer);
     * });
     */
    using SystemProfiler = std::function<void(std::string_view, double)>;

    /**
     * @brief Create a new entity, without any component.
     *
     * @return The new entity.
     */
    [[nodiscard]] Entity createEntity(void);

    /**
     * @brief Create an entity with a given id, without any component, e.g. to load a scene whose entities name
     *        each other by id. The ids below it that no entity has are the next ones createEntity() gives.
     *
     * @param id The id of the new entity
     *
     * @return The new entity.
     *
     * @throw std::invalid_argument if an entity with this id is alive.
     */
    [[nodiscard]] Entity createEntity(Entity::Id id);

    /**
     * @brief Destroy the entity and erase all its components.
     *        During updateSystems(), the entity is destroyed once all the systems have run.
     *
     * @param entity The entity to destroy
     *
     * @throw std::invalid_argument if the entity is not alive.
     */
    void destroyEntity(Entity entity);

    /**
     * @brief Destroy every entity and erase all the components, and keep the systems, e.g. before loading another
     *        scene in the same registry. The next entity created gets the id 0.
     *
     * @throw std::logic_error if called during updateSystems(): a system is still running.
     */
    void destroyEntities(void);

    /**
     * @brief Check if the entity has been created and not destroyed yet.
     *
     * @param entity The entity to check
     *
     * @return true if the entity is alive.
     */
    [[nodiscard]] bool isAlive(Entity entity) const noexcept;

    /**
     * @brief Get all the alive entities, e.g. to list them in an editor.
     *
     * @return The entities, sorted by id.
     */
    [[nodiscard]] std::vector<Entity> getEntities(void) const;

    /**
     * @brief Add a component to the entity, replacing the one of the same type it already has.
     *
     * @tparam Component The type of the component
     * @param entity The entity owning the component
     * @param component The component to add
     *
     * @return The added component.
     *
     * @throw std::invalid_argument if the entity is not alive.
     * @throw std::length_error if there are more than MAX_COMPONENTS component types.
     */
    template <typename Component>
    Component &addComponent(Entity entity, Component component)
    {
        this->checkAlive(entity);
        Component &added = this->getComponents<Component>().insert(entity, std::move(component));

        this->_signatures[entity.getId()].set(this->getComponentType<Component>());
        this->refreshSystems(entity);
        return added;
    }

    /**
     * @brief Remove a component of the entity, does nothing if it has none of this type.
     *
     * @tparam Component The type of the component
     * @param entity The entity owning the component
     *
     * @throw std::invalid_argument if the entity is not alive.
     * @throw std::length_error if there are more than MAX_COMPONENTS component types.
     */
    template <typename Component>
    void removeComponent(Entity entity)
    {
        this->checkAlive(entity);
        this->getComponents<Component>().erase(entity);
        this->_signatures[entity.getId()].reset(this->getComponentType<Component>());
        this->refreshSystems(entity);
    }

    /**
     * @brief Check if the entity has a component of this type.
     *
     * @tparam Component The type of the component
     * @param entity The entity to check
     *
     * @return true if the entity has a component of this type.
     */
    template <typename Component>
    [[nodiscard]] bool hasComponent(Entity entity) const noexcept
    {
        return this->getComponents<Component>().has(entity);
    }

    /**
     * @brief Get a component of the entity.
     *
     * @tparam Component The type of the component
     * @param entity The entity owning the component
     *
     * @return The component of the entity.
     *
     * @throw std::invalid_argument if the entity is not alive.
     * @throw std::out_of_range if the entity has no component of this type.
     */
    template <typename Component>
    [[nodiscard]] Component &getComponent(Entity entity)
    {
        this->checkAlive(entity);
        return this->getComponents<Component>().get(entity);
    }

    /**
     * @brief Get a component of the entity.
     *
     * @tparam Component The type of the component
     * @param entity The entity owning the component
     *
     * @return The component of the entity.
     *
     * @throw std::invalid_argument if the entity is not alive.
     * @throw std::out_of_range if the entity has no component of this type.
     */
    template <typename Component>
    [[nodiscard]] const Component &getComponent(Entity entity) const
    {
        this->checkAlive(entity);
        return this->getComponents<Component>().get(entity);
    }

    /**
     * @brief Get the sparse array of a component type, created if it does not exist yet.
     *
     * @tparam Component The type of the components
     *
     * @return The sparse array of all the components of this type.
     */
    template <typename Component>
    [[nodiscard]] SparseArray<Component> &getComponents(void)
    {
        std::unique_ptr<ISparseArray> &components = this->_components[std::type_index(typeid(Component))];

        if (components == nullptr) {
            components = std::make_unique<SparseArray<Component>>();
        }
        return static_cast<SparseArray<Component> &>(*components);
    }

    /**
     * @brief Get the sparse array of a component type, without creating it.
     *
     * @tparam Component The type of the components
     *
     * @return The sparse array of all the components of this type,
     *         or an empty one if no component of this type has been added yet.
     */
    template <typename Component>
    [[nodiscard]] const SparseArray<Component> &getComponents(void) const noexcept
    {
        const auto it = this->_components.find(std::type_index(typeid(Component)));
        static const SparseArray<Component> empty;

        if (it == this->_components.end()) {
            return empty;
        }
        return static_cast<const SparseArray<Component> &>(*it->second);
    }

    /**
     * @brief Get the alive entities having all these components, outside of a system,
     *        e.g. to draw the sprites once per frame while the systems run once per fixed step.
     *        It goes through every entity: a system is cheaper for code run at each step.
     *
     * @tparam Components The components the entities must have, at least one
     *
     * @return The entities, sorted by id, empty if one of the types was never added to an entity.
     */
    template <typename... Components>
    [[nodiscard]] std::vector<Entity> getEntitiesWith(void) const
    {
        static_assert(sizeof...(Components) > 0, "getEntitiesWith needs at least one component type");
        const std::array<std::optional<std::size_t>, sizeof...(Components)> types{this->findComponentType<Components>()...};
        Signature signature;
        std::vector<Entity> entities;

        for (const std::optional<std::size_t> &type : types) {
            if (!type) {
                return entities;
            }
            signature.set(*type);
        }
        for (Entity::Id id = 0; id < this->_alive.size(); ++id) {
            if (this->_alive[id] && (this->_signatures[id] & signature) == signature) {
                entities.emplace_back(id);
            }
        }
        return entities;
    }

    /**
     * @brief Add a system, run by updateSystems() after the systems added before it.
     *        Its entities are the ones having all the components listed after its type,
     *        e.g. addSystem<MovementSystem, Position, Velocity>().
     *
     * @tparam SystemType The type of the system, deriving from System
     * @tparam Components The components an entity needs to be in the system
     * @param args The arguments given to the constructor of the system
     *
     * @return The added system.
     *
     * @throw std::logic_error if called during updateSystems(): the systems are running.
     * @throw std::length_error if there are more than MAX_COMPONENTS component types.
     */
    template <std::derived_from<System> SystemType, typename... Components, typename... Args>
    SystemType &addSystem(Args &&...args)
    {
        if (this->_isUpdating) {
            throw std::logic_error("younity::ecs::Registry: cannot add a system during updateSystems");
        }
        Signature signature;

        (signature.set(this->getComponentType<Components>()), ...);
        std::unique_ptr<SystemType> system = std::make_unique<SystemType>(std::forward<Args>(args)...);

        if (system->getName().empty()) {
            system->setName(defaultSystemName(typeid(SystemType)));
        }
        SystemType &added = *system;

        this->_systems.push_back(SystemEntry{signature, std::move(system), {}});

        // The entities created before the system
        for (Entity::Id id = 0; id < this->_alive.size(); ++id) {
            if (this->_alive[id]) {
                this->refreshSystem(this->_systems.back(), Entity(id));
            }
        }
        return added;
    }

    /**
     * @brief Run all the systems, in the order they have been added.
     *        If a system or the profiler throws, the entities queued for destruction are destroyed
     *        and the update state is cleared before the exception is rethrown.
     *
     * @param dt The time elapsed since the last frame, in seconds
     */
    void updateSystems(float dt);

    /**
     * @brief Set the function called after each system runs, with its name and its duration.
     *        It is called once per system per updateSystems(), after the system has run.
     *
     * @param profiler The function to call, or an empty one to stop profiling
     */
    void setSystemProfiler(SystemProfiler profiler);

    /**
     * @brief Remove every entity, component and system.
     *        The next entity created gets the id 0, and the component type numbers start again.
     *
     * @throw std::logic_error if called during updateSystems(): a system is still running.
     */
    void clear(void);

private:
    /**
     * @brief A system, the signature of the components it needs and its entities.
     */
    struct SystemEntry
    {
        Signature signature;
        std::unique_ptr<System> system;
        std::set<Entity> entities;
    };

    /**
     * @brief Get the name of a system type, without its namespaces, e.g. "MovementSystem".
     *
     * @param type The type of the system, from typeid()
     *
     * @return The demangled type name, or the raw name if it cannot be demangled.
     */
    [[nodiscard]] static std::string defaultSystemName(const std::type_info &type);

    /**
     * @brief Check that the entity is alive.
     *
     * @param entity The entity to check
     *
     * @throw std::invalid_argument if the entity is not alive.
     */
    void checkAlive(Entity entity) const;

    /**
     * @brief Leave the update state and destroy the entities queued during the update.
     */
    void finishUpdate(void);

    /**
     * @brief Add the entity to the systems its signature matches, and remove it from the others.
     *
     * @param entity The entity whose signature changed
     */
    void refreshSystems(Entity entity);

    /**
     * @brief Add the entity to the system if its signature matches, remove it otherwise.
     *
     * @param entry The system to update
     * @param entity The entity to check
     */
    void refreshSystem(SystemEntry &entry, Entity entity);

    /**
     * @brief Get the number of a component type, given the first time the type is used.
     *
     * @tparam Component The type of the component
     *
     * @return The number of the component type, i.e. its bit in the signatures.
     *
     * @throw std::length_error if there are more than MAX_COMPONENTS component types.
     */
    template <typename Component>
    [[nodiscard]] std::size_t getComponentType(void)
    {
        const std::type_index type(typeid(Component));
        const auto it = this->_componentTypes.find(type);

        if (it != this->_componentTypes.end()) {
            return it->second;
        }
        if (this->_componentTypes.size() == MAX_COMPONENTS) {
            throw std::length_error("younity::ecs::Registry: too many component types");
        }
        const std::size_t number = this->_componentTypes.size();

        this->_componentTypes.emplace(type, number);
        return number;
    }

    /**
     * @brief Get the number of a component type without giving it one, for the const functions.
     *
     * @tparam Component The type of the component
     *
     * @return The number of the component type, or nothing if the type was never used.
     */
    template <typename Component>
    [[nodiscard]] std::optional<std::size_t> findComponentType(void) const
    {
        const auto it = this->_componentTypes.find(std::type_index(typeid(Component)));

        if (it == this->_componentTypes.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    std::vector<bool> _alive;
    std::vector<Signature> _signatures;
    std::vector<Entity::Id> _freeIds;
    std::unordered_map<std::type_index, std::unique_ptr<ISparseArray>> _components;
    std::unordered_map<std::type_index, std::size_t> _componentTypes;
    std::vector<SystemEntry> _systems;
    SystemProfiler _systemProfiler;
    bool _isUpdating = false;
    std::vector<Entity> _pendingDestroys;
};

} // namespace younity::ecs

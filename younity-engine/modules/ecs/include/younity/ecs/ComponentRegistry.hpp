/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/ComponentRegistry
** File description:
** ComponentRegistry header file
*/

#pragma once

#include <concepts>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include "younity/ecs/ComponentInfo.hpp"

namespace younity::ecs {

/**
 * @brief The component types known by their name, e.g. to save a scene to a file, show the components
 *        in an editor or reach them from a script. One is shared by all the registries of the game.
 */
class ComponentRegistry
{
public:
    /**
     * @brief Register a component type under a name, then its fields with ComponentInfo::field(), e.g.
     *        components.add<Transform>("Transform").field("position", &Transform::position).
     *
     * @tparam Component The type of the component, built with its default values when added by name
     * @param name The name of the component
     *
     * @return The information of the component type, to add its fields.
     *
     * @throw std::invalid_argument if the name or the type is already registered.
     */
    template <std::default_initializable Component>
    ComponentInfo &add(std::string name)
    {
        const std::type_index type(typeid(Component));

        if (this->_components.contains(name)) {
            throw std::invalid_argument("younity::ecs::ComponentRegistry: the component " + name + " already exists");
        }
        for (const auto &[registered, info] : this->_components) {
            if (info.getType() == type) {
                throw std::invalid_argument("younity::ecs::ComponentRegistry: the type is already registered as " + registered);
            }
        }
        ComponentInfo info = ComponentInfo::create<Component>(name);

        return this->_components.emplace(std::move(name), std::move(info)).first->second;
    }

    /**
     * @brief Get a component type by its name.
     *
     * @param name The name of the component
     *
     * @return The information of the component type.
     *
     * @throw std::out_of_range if no component type has this name.
     */
    [[nodiscard]] const ComponentInfo &get(std::string_view name) const;

    /**
     * @brief Get the names of all the registered component types.
     *
     * @return The names, sorted alphabetically.
     */
    [[nodiscard]] std::vector<std::string> getNames(void) const;

    /**
     * @brief Get the names of the registered component types the entity has.
     *        Only the component types registered here are listed.
     *
     * @param registry The registry owning the entity
     * @param entity The entity to check
     *
     * @return The names, sorted alphabetically.
     *
     * @throw std::invalid_argument if the entity is not alive.
     */
    [[nodiscard]] std::vector<std::string> getNamesOf(const Registry &registry, Entity entity) const;

private:
    std::map<std::string, ComponentInfo, std::less<>> _components;
};

} // namespace younity::ecs

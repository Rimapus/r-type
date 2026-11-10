/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/src/ComponentRegistry
** File description:
** ComponentRegistry source file
*/

#include "younity/ecs/ComponentRegistry.hpp"

namespace younity::ecs {

const ComponentInfo &ComponentRegistry::get(std::string_view name) const
{
    const auto it = this->_components.find(name);

    if (it == this->_components.end()) {
        throw std::out_of_range("younity::ecs::ComponentRegistry: no component is named " + std::string(name));
    }
    return it->second;
}

std::vector<std::string> ComponentRegistry::getNames(void) const
{
    std::vector<std::string> names;

    names.reserve(this->_components.size());
    for (const auto &[name, info] : this->_components) {
        names.push_back(name);
    }
    return names;
}

std::vector<std::string> ComponentRegistry::getNamesOf(const Registry &registry, Entity entity) const
{
    std::vector<std::string> names;

    if (!registry.isAlive(entity)) {
        throw std::invalid_argument("younity::ecs::ComponentRegistry: the entity is not alive");
    }
    for (const auto &[name, info] : this->_components) {
        if (info.has(registry, entity)) {
            names.push_back(name);
        }
    }
    return names;
}

} // namespace younity::ecs

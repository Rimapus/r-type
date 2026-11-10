/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/src/Registry
** File description:
** Registry source file
*/

#include "younity/ecs/Registry.hpp"
#include "younity/core/Clock.hpp"

#if defined(__has_include)
#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#define YOUNITY_HAS_CXA_DEMANGLE
#endif
#endif

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace younity::ecs {

Entity Registry::createEntity(void)
{
    if (this->_freeIds.empty() == false) {
        const Entity::Id id = this->_freeIds.back();

        this->_freeIds.pop_back();
        this->_alive[id] = true;
        this->refreshSystems(Entity(id));
        return Entity(id);
    }
    const auto id = static_cast<Entity::Id>(this->_alive.size());

    this->_alive.push_back(true);
    this->_signatures.emplace_back();
    this->refreshSystems(Entity(id));
    return Entity(id);
}

Entity Registry::createEntity(Entity::Id id)
{
    if (this->isAlive(Entity(id))) {
        throw std::invalid_argument("younity::ecs::Registry: an entity with this id is alive");
    }
    if (id < this->_alive.size()) {
        std::erase(this->_freeIds, id);
    } else {
        for (auto free = static_cast<Entity::Id>(this->_alive.size()); free < id; ++free) {
            this->_freeIds.push_back(free);
        }
        this->_alive.resize(static_cast<std::size_t>(id) + 1, false);
        this->_signatures.resize(static_cast<std::size_t>(id) + 1);
    }
    this->_alive[id] = true;
    this->refreshSystems(Entity(id));
    return Entity(id);
}

void Registry::destroyEntity(Entity entity)
{
    this->checkAlive(entity);

    // A system may still use the entity during this frame: destroy it once all the systems have run
    if (this->_isUpdating) {
        this->_pendingDestroys.push_back(entity);
        return;
    }

    for (const auto &entry : this->_components) {
        entry.second->erase(entity);
    }
    for (SystemEntry &entry : this->_systems) {
        entry.entities.erase(entity);
    }
    this->_signatures[entity.getId()].reset();
    this->_alive[entity.getId()] = false;
    this->_freeIds.push_back(entity.getId());
}

bool Registry::isAlive(Entity entity) const noexcept
{
    return entity.getId() < this->_alive.size() && this->_alive[entity.getId()];
}

std::vector<Entity> Registry::getEntities(void) const
{
    std::vector<Entity> entities;

    entities.reserve(this->_alive.size());
    for (Entity::Id id = 0; id < this->_alive.size(); ++id) {
        if (this->_alive[id]) {
            entities.emplace_back(id);
        }
    }
    return entities;
}

void Registry::updateSystems(float dt)
{
    this->_isUpdating = true;
    try {
        for (SystemEntry &entry : this->_systems) {
            const std::vector<Entity> entities(entry.entities.begin(), entry.entities.end());

            if (!this->_systemProfiler) {
                entry.system->update(*this, entities, dt);
                continue;
            }
            const Clock clock;

            entry.system->update(*this, entities, dt);
            this->_systemProfiler(entry.system->getName(), clock.getElapsedTime());
        }
    } catch (...) {
        this->finishUpdate();
        throw;
    }
    this->finishUpdate();
}

void Registry::finishUpdate(void)
{
    this->_isUpdating = false;
    for (const Entity entity : this->_pendingDestroys) {
        if (this->isAlive(entity)) {
            this->destroyEntity(entity);
        }
    }
    this->_pendingDestroys.clear();
}

void Registry::setSystemProfiler(SystemProfiler profiler)
{
    this->_systemProfiler = std::move(profiler);
}

std::string Registry::defaultSystemName(const std::type_info &type)
{
    std::string name = type.name();

#ifdef YOUNITY_HAS_CXA_DEMANGLE
    int status = 0;
    char *demangled = abi::__cxa_demangle(type.name(), nullptr, nullptr, &status);

    if (demangled != nullptr && status == 0) {
        name = demangled;
    }
    std::free(demangled);
#endif
    const std::size_t templateStart = name.find('<');

    // Keep the class alone: the namespaces and the template arguments are noise in a metric name
    if (templateStart != std::string::npos) {
        name.resize(templateStart);
    }
    const std::size_t separator = name.rfind("::");

    return separator == std::string::npos ? name : name.substr(separator + 2);
}

void Registry::destroyEntities(void)
{
    if (this->_isUpdating) {
        throw std::logic_error("younity::ecs::Registry: cannot destroy the entities during updateSystems");
    }
    for (const Entity entity : this->getEntities()) {
        this->destroyEntity(entity);
    }
    this->_alive.clear();
    this->_signatures.clear();
    this->_freeIds.clear();
}

void Registry::clear(void)
{
    if (this->_isUpdating) {
        throw std::logic_error("younity::ecs::Registry: cannot clear the registry during updateSystems");
    }
    this->_alive.clear();
    this->_signatures.clear();
    this->_freeIds.clear();
    this->_components.clear();
    this->_componentTypes.clear();
    this->_systems.clear();
    this->_pendingDestroys.clear();
}

void Registry::checkAlive(Entity entity) const
{
    if (!this->isAlive(entity)) {
        throw std::invalid_argument("younity::ecs::Registry: the entity is not alive");
    }
}

void Registry::refreshSystems(Entity entity)
{
    for (SystemEntry &entry : this->_systems) {
        this->refreshSystem(entry, entity);
    }
}

void Registry::refreshSystem(SystemEntry &entry, Entity entity)
{
    const Signature &signature = this->_signatures[entity.getId()];

    if ((signature & entry.signature) == entry.signature) {
        entry.entities.insert(entity);
    } else {
        entry.entities.erase(entity);
    }
}

} // namespace younity::ecs

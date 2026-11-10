/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/src/Scene
** File description:
** Scene source file
*/

#include "younity/ecs/Scene.hpp"

namespace younity::ecs {

void Scene::update(float dt)
{
    this->_registry.updateSystems(dt);
}

Registry &Scene::getRegistry(void) noexcept
{
    return this->_registry;
}

EventBus &Scene::getEventBus(void) noexcept
{
    return this->_bus;
}

} // namespace younity::ecs

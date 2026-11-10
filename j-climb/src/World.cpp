/*
** EPITECH PROJECT, 2026
** j-climb/src/World
** File description:
** World source file
*/

#include "World.hpp"

#include <expected>
#include <stdexcept>

#include "younity/ecs/Components.hpp"
#include "younity/ecs/SceneFile.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/ecs/systems/MovementSystem.hpp"
#include "younity/physics/Components.hpp"
#include "younity/physics/components/Hitbox.hpp"
#include "younity/physics/systems/CollisionSystem.hpp"
#include "younity/render/Components.hpp"
#include "younity/scripting/Components.hpp"
#include "younity/scripting/components/Script.hpp"
#include "younity/scripting/systems/ScriptSystem.hpp"

namespace jclimb {

World::World(younity::assets::ResourceManager &resources, const younity::scripting::Host &host,
             const std::filesystem::path &scene)
{
    younity::ecs::registerComponents(this->_components);
    younity::physics::registerComponents(this->_components);
    younity::render::registerComponents(this->_components, resources);
    younity::scripting::registerComponents(this->_components);
    // The scripts move the player and decide what the collisions mean, so the gravity and the solids of the engine
    // are left out: the rain and the particles only need their speed
    this->_registry.addSystem<younity::scripting::ScriptSystem, younity::scripting::Script>(
        this->_components, this->_bus, host, resources.getRoot());
    this->_registry.addSystem<younity::ecs::MovementSystem, younity::ecs::Transform, younity::ecs::Velocity>();
    this->_registry.addSystem<younity::physics::CollisionSystem, younity::ecs::Transform, younity::physics::Hitbox>(
        this->_bus);

    const std::expected<void, younity::ecs::SceneFileError> loaded =
        younity::ecs::loadScene(this->_registry, this->_components, resources.getRoot() / scene);

    if (!loaded) {
        throw std::runtime_error("cannot load " + loaded.error().path.generic_string() + ": " +
                                 loaded.error().message);
    }
}

void World::update(float step)
{
    this->_registry.updateSystems(step);
}

const younity::ecs::Registry &World::getRegistry(void) const noexcept
{
    return this->_registry;
}

} // namespace jclimb

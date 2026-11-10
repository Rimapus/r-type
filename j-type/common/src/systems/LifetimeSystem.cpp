/*
** EPITECH PROJECT, 2026
** j-type/common/src/systems/LifetimeSystem
** File description:
** LifetimeSystem source file
*/

#include "jtype/common/systems/LifetimeSystem.hpp"

#include "jtype/common/components/Lifetime.hpp"
#include "younity/ecs/Registry.hpp"

namespace jtype {

void LifetimeSystem::update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt)
{
    for (const younity::ecs::Entity entity : entities) {
        Lifetime &lifetime = registry.getComponent<Lifetime>(entity);

        lifetime.remaining -= dt;
        if (lifetime.remaining <= 0.0F) {
            registry.destroyEntity(entity);
        }
    }
}

} // namespace jtype

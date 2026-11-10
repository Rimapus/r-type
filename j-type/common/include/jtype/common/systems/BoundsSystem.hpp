/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/systems/BoundsSystem
** File description:
** BoundsSystem header file
*/

#pragma once

#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace jtype {

/**
 * @brief Destroys the missiles once they left the screen by any side, and the Bydos, the power-ups and the walls and the
 *        zappers once they left it on the left. The PlayerMovementSystem keeps the players on it.
 *        Add it with registry.addSystem<BoundsSystem, Transform>(), after the MovementSystem.
 */
class BoundsSystem : public younity::ecs::System
{
public:
    void update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt) override;
};

} // namespace jtype

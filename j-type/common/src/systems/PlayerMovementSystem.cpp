/*
** EPITECH PROJECT, 2026
** j-type/common/src/systems/PlayerMovementSystem
** File description:
** PlayerMovementSystem source file
*/

#include "jtype/common/systems/PlayerMovementSystem.hpp"

#include <algorithm>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Input.hpp"
#include "jtype/common/components/Player.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"

namespace jtype {

younity::Vector2f moveShip(younity::Vector2f position, std::uint8_t inputs, float dt)
{
    younity::Vector2f direction{};

    direction.x = static_cast<float>((inputs & input::RIGHT) != 0) - static_cast<float>((inputs & input::LEFT) != 0);
    direction.y = static_cast<float>((inputs & input::DOWN) != 0) - static_cast<float>((inputs & input::UP) != 0);
    // Normalized: a diagonal is as fast as a straight line
    position += direction.normalized() * PLAYER_SPEED * dt;
    position.x = std::clamp(position.x, PLAYER_ORIGIN.x, WORLD_WIDTH - PLAYER_SIZE.x + PLAYER_ORIGIN.x);
    position.y = std::clamp(position.y, PLAYER_ORIGIN.y, WORLD_HEIGHT - PLAYER_SIZE.y + PLAYER_ORIGIN.y);
    return position;
}

void PlayerMovementSystem::update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt)
{
    for (const younity::ecs::Entity entity : entities) {
        younity::Vector2f &position = registry.getComponent<younity::ecs::Transform>(entity).position;

        position = moveShip(position, registry.getComponent<Player>(entity).inputs, dt);
    }
}

} // namespace jtype

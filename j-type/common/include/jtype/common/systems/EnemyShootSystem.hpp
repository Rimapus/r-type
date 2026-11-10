/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/systems/EnemyShootSystem
** File description:
** EnemyShootSystem header file
*/

#pragma once

#include <vector>

#include "jtype/common/Difficulty.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace jtype {

/**
 * @brief Makes the Bydos that canShoot() fire back, ENEMY_FIRST_SHOT_DELAY after they enter the screen, then once per
 *        ENEMY_SHOOT_DELAY: a SentryDrone aims at the nearest player, a Jelly shoots straight to the left. Runs on the
 *        server.
 *        Add it with registry.addSystem<EnemyShootSystem, Enemy, Transform>().
 */
class EnemyShootSystem : public younity::ecs::System
{
public:
    /**
     * @brief Create the system, with the speed and the rate of the shots of a difficulty.
     *
     * @param rules The rules, read at each step so a change applies at once; they must outlive the system
     */
    explicit EnemyShootSystem(const DifficultyRules &rules = NORMAL_RULES);

    /**
     * @brief Count down the cooldown of each Bydo that shoots and is on screen, and fire a missile from it once the
     *        cooldown is over.
     *
     * @param registry The registry owning the Bydos and the players
     * @param entities The entities having an Enemy and a Transform
     * @param dt The duration of the step, in seconds
     */
    void update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt) override;

private:
    const DifficultyRules &_rules;
};

} // namespace jtype

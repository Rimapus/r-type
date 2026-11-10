/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/systems/BossSystem
** File description:
** BossSystem header file
*/

#pragma once

#include <vector>

#include "jtype/common/Difficulty.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace jtype {

/**
 * @brief Moves and fires the boss: its body comes in from the right at BOSS_ENTER_SPEED until its right side is on the
 *        right edge, then rises and falls around the middle of the screen, with its eye on it. At the edge it fires an
 *        orb at the nearest player every BOSS_ORB_DELAY and a fan of BOSS_SPIT_COUNT spits every BOSS_SPIT_DELAY, faster
 *        once the eye has lost half of its health. Runs on the server.
 *        Add it with registry.addSystem<BossSystem, Boss, Transform>().
 */
class BossSystem : public younity::ecs::System
{
public:
    /**
     * @brief Create the system, with the speed and the rate of the shots of a difficulty.
     *
     * @param rules The rules, read at each step so a change applies at once; they must outlive the system
     */
    explicit BossSystem(const DifficultyRules &rules = NORMAL_RULES);

    /**
     * @brief Move the body and the eye of the boss, and fire the attacks that are due.
     *
     * @param registry The registry owning the boss and the players
     * @param entities The entities having a Boss and a Transform: the parts of the boss
     * @param dt The duration of the step, in seconds
     */
    void update(younity::ecs::Registry &registry, const std::vector<younity::ecs::Entity> &entities, float dt) override;

private:
    const DifficultyRules &_rules;
};

} // namespace jtype

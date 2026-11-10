/*
** EPITECH PROJECT, 2026
** j-type/tests/common/Components
** File description:
** Unit tests of the game components
*/

#include <criterion/criterion.h>

#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Health.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/components/Score.hpp"

using jtype::Enemy;
using jtype::EnemyType;
using jtype::Health;
using jtype::Missile;
using jtype::MovementPattern;
using jtype::Player;
using jtype::Score;

Test(common_components, missile_of_a_player_names_its_shooter)
{
    const Missile missile{2};

    cr_assert(missile.shooter.has_value());
    cr_assert_eq(*missile.shooter, 2);
    cr_assert_eq(missile.damage, 1);
}

Test(common_components, missile_of_a_bydo_has_no_shooter)
{
    const Missile missile{};

    cr_assert_not(missile.shooter.has_value());
}

Test(common_components, enemy_is_a_straight_blobling_by_default)
{
    const Enemy enemy{};

    cr_assert(enemy.type == EnemyType::Blobling);
    cr_assert(enemy.pattern == MovementPattern::Straight);
}

Test(common_components, new_components_start_empty)
{
    cr_assert_eq(Player{}.inputs, 0);
    cr_assert_eq(Health{}.current, Health{}.max);
    cr_assert_eq(Score{}.points, 0U);
}

/*
** EPITECH PROJECT, 2026
** j-type/tests/common/EntityType
** File description:
** Unit tests of the entity types
*/

#include <criterion/criterion.h>

#include <cstdint>

#include "jtype/common/EntityType.hpp"

using jtype::EntityType;
using jtype::getMissileType;
using jtype::getPlayerSlot;

Test(common_entity_type, each_player_has_its_missile)
{
    cr_assert(getMissileType(0) == EntityType::Missile1);
    cr_assert(getMissileType(1) == EntityType::Missile2);
    cr_assert(getMissileType(2) == EntityType::Missile3);
    cr_assert(getMissileType(3) == EntityType::Missile4);
}

Test(common_entity_type, each_type_has_the_value_of_the_protocol)
{
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::Player1), 0);
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::Player4), 3);
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::Missile1), 4);
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::Missile4), 7);
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::EnemyMissile), 8);
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::Blobling), 9);
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::Jelly), 10);
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::SentryDrone), 11);
    cr_assert_eq(static_cast<std::uint8_t>(EntityType::Explosion), 12);
}

Test(common_entity_type, ships_give_their_player)
{
    cr_assert_eq(getPlayerSlot(EntityType::Player1).value_or(255), 0);
    cr_assert_eq(getPlayerSlot(EntityType::Player4).value_or(255), 3);
    cr_assert_not(getPlayerSlot(EntityType::Missile1).has_value());
    cr_assert_not(getPlayerSlot(EntityType::Blobling).has_value());
}

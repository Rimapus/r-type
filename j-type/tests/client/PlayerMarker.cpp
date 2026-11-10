/*
** EPITECH PROJECT, 2026
** j-type/tests/client/PlayerMarker
** File description:
** Unit tests of the markers of the ships
*/

#include <criterion/criterion.h>

#include <optional>

#include "jtype/common/EntityType.hpp"
#include "render/PlayerMarker.hpp"

using jtype::EntityType;
using jtype::FontColor;
using jtype::layoutPlayerMarker;
using jtype::PlayerMarker;

Test(client_player_marker, ships_have_the_tag_of_their_player)
{
    const std::optional<PlayerMarker> marker = layoutPlayerMarker(EntityType::Player3, std::nullopt);

    cr_assert(marker.has_value());
    cr_assert_str_eq(marker->tag.c_str(), "player_tag_p3");
    // Above the head of the pilot, on the right of the ship
    cr_assert_lt(marker->tagOffset.y, 0.0F);
    cr_assert_gt(marker->tagOffset.x, 0.0F);
    cr_assert_not(marker->label.has_value());
}

Test(client_player_marker, only_the_ship_of_this_client_has_you)
{
    const std::optional<PlayerMarker> mine = layoutPlayerMarker(EntityType::Player2, 1);
    const std::optional<PlayerMarker> other = layoutPlayerMarker(EntityType::Player1, 1);

    cr_assert(mine.has_value() && mine->label.has_value());
    cr_assert_str_eq(mine->label->string.c_str(), "YOU");
    cr_assert(mine->label->color == FontColor::Amber);
    // Under the ship
    cr_assert_gt(mine->label->position.y, 0.0F);
    cr_assert(other.has_value());
    cr_assert_not(other->label.has_value());
}

Test(client_player_marker, other_entities_have_no_marker)
{
    cr_assert_not(layoutPlayerMarker(EntityType::Missile2, 1).has_value());
    cr_assert_not(layoutPlayerMarker(EntityType::Jelly, std::nullopt).has_value());
}

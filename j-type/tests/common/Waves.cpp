/*
** EPITECH PROJECT, 2026
** j-type/tests/common/Waves
** File description:
** Unit tests of the wave files
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "jtype/common/Waves.hpp"

using jtype::EnemyType;
using jtype::Formation;
using jtype::MovementPattern;
using jtype::Wave;
using jtype::WaveGroup;

namespace {

auto parse(const std::string &text)
{
    std::istringstream input(text);

    return jtype::parseWaves(input);
}

} // namespace

Test(common_waves, reads_the_waves_and_their_groups)
{
    const auto waves = parse("# a comment\n"
                             "wave 2\n"
                             "blobling straight 5 0.8\n"
                             "\n"
                             "wave 3.5\n"
                             "jelly sine_wave 3 1.2\n"
                             "sentry_drone chase 1 0\n");

    cr_assert(waves.has_value());
    cr_assert(*waves == (std::vector<Wave>{
                            Wave{2.0F, {WaveGroup{EnemyType::Blobling, MovementPattern::Straight, 5, 0.8F}}},
                            Wave{3.5F,
                                 {WaveGroup{EnemyType::Jelly, MovementPattern::SineWave, 3, 1.2F},
                                  WaveGroup{EnemyType::SentryDrone, MovementPattern::Chase, 1, 0.0F}}      },
    }));
}

Test(common_waves, reads_the_formation_and_the_height)
{
    const auto waves = parse("wave 1\n"
                             "blobling straight 4 0 column 0.25\n"
                             "jelly sine_wave 5 0 v 1\n"
                             "blobling straight 3 0.5 line 0\n"
                             "sentry_drone chase 2 1 random\n");

    cr_assert(waves.has_value(), "%s", waves ? "" : waves.error().c_str());
    cr_assert(waves->front().groups == (std::vector<WaveGroup>{
                                           WaveGroup{EnemyType::Blobling,    MovementPattern::Straight, 4, 0.0F, Formation::Column, 0.25F},
                                           WaveGroup{EnemyType::Jelly,       MovementPattern::SineWave, 5, 0.0F, Formation::V,      1.0F },
                                           WaveGroup{EnemyType::Blobling,    MovementPattern::Straight, 3, 0.5F, Formation::Line,   0.0F },
                                           WaveGroup{EnemyType::SentryDrone, MovementPattern::Chase,    2, 1.0F, Formation::Random, 0.5F },
    }));
}

Test(common_waves, group_without_formation_is_random)
{
    const auto waves = parse("wave 1\nblobling straight 5 0.8\n");

    cr_assert(waves.has_value());
    cr_assert(waves->front().groups.front().formation == Formation::Random);
}

Test(common_waves, rejects_wrong_formations)
{
    cr_assert(parse("wave 1\nblobling straight 1 1 circle 0.5\n").error().starts_with("line 2: unknown formation"));
    cr_assert(parse("wave 1\nblobling straight 1 1 line\n").error().starts_with("line 2: a formation other than random needs a height"));
    cr_assert_not(parse("wave 1\nblobling straight 1 1 line 1.5\n").has_value());
    cr_assert_not(parse("wave 1\nblobling straight 1 1 line -0.1\n").has_value());
    cr_assert_not(parse("wave 1\nblobling straight 1 1 random 0.5\n").has_value());
    cr_assert_not(parse("wave 1\nblobling straight 1 1 line 0.5 extra\n").has_value());
}

Test(common_waves, reads_the_drop_of_a_group)
{
    const auto waves = parse("wave 1\n"
                             "blobling straight 5 0 v 0.5 drop\n"
                             "jelly straight 3 1 drop\n"
                             "jelly straight 3 1 random drop\n"
                             "sentry_drone chase 1 0\n");

    cr_assert(waves.has_value(), "%s", waves ? "" : waves.error().c_str());

    const std::vector<WaveGroup> &groups = waves->front().groups;

    cr_assert(groups[0].drop);
    cr_assert(groups[0].formation == Formation::V);
    cr_assert(groups[1].drop);
    cr_assert(groups[2].drop);
    cr_assert(groups[2].formation == Formation::Random);
    cr_assert_not(groups[3].drop);
    cr_assert_not(parse("wave 1\nblobling straight 1 1 drop line 0.5\n").has_value());
    cr_assert_not(parse("wave 1\nblobling straight 1 1 drop drop\n").has_value());
}

Test(common_waves, reads_the_obstacles_of_a_wave)
{
    const auto waves = parse("wave 1\nwall 0 0.4\nzapper 0.6 1\n");

    cr_assert(waves.has_value(), "%s", waves ? "" : waves.error().c_str());
    cr_assert(waves->front().obstacles == (std::vector<jtype::WaveObstacle>{
                                              jtype::WaveObstacle{jtype::ObstacleKind::Block,      0.0F, 0.4F},
                                              jtype::WaveObstacle{jtype::ObstacleKind::ZapperNode, 0.6F, 1.0F},
    }));
}

Test(common_waves, rejects_wrong_obstacles)
{
    cr_assert(parse("wave 1\nwall 0.6 0.2\n").error().starts_with("line 2: the heights of an obstacle"));
    cr_assert_not(parse("wave 1\nwall 0 1.5\n").has_value());
    cr_assert_not(parse("wave 1\nzapper 0.5\n").has_value());
    cr_assert_not(parse("wave 1\nzapper 0 1 high\n").has_value());
    cr_assert_not(parse("wall 0 1\n").has_value());
}

Test(common_waves, reads_the_boss_of_a_wave)
{
    const auto waves = parse("wave 1\nblobling straight 2 0.5\n\nwave 4\nboss incubator\n");

    cr_assert(waves.has_value(), "%s", waves ? "" : waves.error().c_str());
    cr_assert_not(waves->front().boss.has_value());
    cr_assert(waves->back().boss == jtype::BossType::Incubator);
    cr_assert(waves->back().groups.empty());
}

Test(common_waves, rejects_wrong_bosses)
{
    cr_assert(parse("wave 1\nboss dragon\n").error().starts_with("line 2: unknown boss"));
    cr_assert(parse("wave 1\nboss incubator\nboss incubator\n").error().starts_with("line 3: a wave has one boss"));
    cr_assert_not(parse("boss incubator\n").has_value());
    cr_assert_not(parse("wave 1\nboss\n").has_value());
    cr_assert_not(parse("wave 1\nboss incubator now\n").has_value());
}

Test(common_waves, tells_the_line_of_an_error)
{
    const auto waves = parse("wave 1\nblobling straight 2 0.5\ndragon straight 1 1\n");

    cr_assert_not(waves.has_value());
    cr_assert(waves.error().starts_with("line 3: unknown type"));
}

Test(common_waves, rejects_wrong_lines)
{
    cr_assert_not(parse("blobling straight 1 1\n").has_value());
    cr_assert_not(parse("wave -1\n").has_value());
    cr_assert_not(parse("wave 1\nblobling zigzag 1 1\n").has_value());
    cr_assert_not(parse("wave 1\nblobling straight 0 1\n").has_value());
    cr_assert_not(parse("wave 1\nblobling straight 2\n").has_value());
    cr_assert_not(parse("# nothing\n").has_value());
}

Test(common_waves, game_file_is_valid)
{
    const auto waves = jtype::loadWaves(std::filesystem::path(JTYPE_SOURCE_DIR) / "assets" / "data" / "waves.txt");

    cr_assert(waves.has_value(), "%s", waves ? "" : waves.error().c_str());
}

Test(common_waves, missing_file_is_an_error)
{
    cr_assert_not(jtype::loadWaves("missing/waves.txt").has_value());
}

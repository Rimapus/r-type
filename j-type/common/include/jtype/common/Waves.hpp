/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Waves
** File description:
** Waves header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <istream>
#include <optional>
#include <string>
#include <vector>

#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Obstacle.hpp"

namespace jtype {

/**
 * @brief Where the Bydos of a group appear, around the height of the group.
 */
enum class Formation : std::uint8_t {
    // Each Bydo at a random height, the height of the group left out
    Random,
    // All at the height of the group: with a delay, one behind the other
    Line,
    // Stacked from top to bottom, centered on the height of the group: with no delay, a wall
    Column,
    // The first one at the height of the group, the next ones by pairs above and below it, each pair further back
    V,
};

/**
 * @brief Bydos of the same kind coming one after the other in a wave.
 */
struct WaveGroup
{
    EnemyType type = EnemyType::Blobling;
    MovementPattern pattern = MovementPattern::Straight;
    std::size_t count = 1;
    // Seconds between two Bydos of the group
    float delay = 1.0F;
    Formation formation = Formation::Random;
    // From 0, the top of the screen, to 1, the bottom
    float height = 0.5F;
    // The last Bydo of the group carries a power-up, chosen at random
    bool drop = false;

    bool operator==(const WaveGroup &other) const = default;
};

/**
 * @brief A wall of blocks or a zapper of a wave, standing between two heights.
 */
struct WaveObstacle
{
    // Block for a wall, ZapperNode for a zapper
    ObstacleKind kind = ObstacleKind::Block;
    // From 0, against the ceiling, to 1, against the floor, the top one first
    float top = 0.0F;
    float bottom = 1.0F;

    bool operator==(const WaveObstacle &other) const = default;
};

/**
 * @brief A wave of Bydos: its groups, its obstacles and its boss come at the same time, after a pause.
 */
struct Wave
{
    // Seconds of calm before the wave, once the Bydos of the wave before are destroyed or gone
    float pause = 0.0F;
    std::vector<WaveGroup> groups;
    // Beating it wins the game
    std::optional<BossType> boss = std::nullopt;
    // They do not hold the next wave: they scroll away with the stage
    std::vector<WaveObstacle> obstacles = {};

    bool operator==(const Wave &other) const = default;
};

/**
 * @brief Read waves written as text, the format of assets/data/waves.txt: "wave <pause>" starts a wave,
 *        each next line "<type> <pattern> <count> <delay> [<formation> <height>] [drop]" adds a group to it, e.g.
 *        "blobling straight 4 0 column 0.3 drop". The formation is random, line, column or v, random by default, and
 *        every formation but random takes a height from 0 to 1. With drop, the last Bydo of the group carries a
 *        power-up. A line "boss incubator" brings the boss with the wave, and "wall <top> <bottom>" or
 *        "zapper <top> <bottom>" an obstacle between two heights from 0, the ceiling, to 1, the floor.
 *        Empty lines and the lines starting with # are skipped.
 *
 * @param input The text
 *
 * @return The waves, or the line and the reason of the first error.
 */
[[nodiscard]] std::expected<std::vector<Wave>, std::string> parseWaves(std::istream &input);

/**
 * @brief Read the waves of a file, see parseWaves().
 *
 * @param path The file
 *
 * @return The waves, or why the file cannot be read.
 */
[[nodiscard]] std::expected<std::vector<Wave>, std::string> loadWaves(const std::filesystem::path &path);

} // namespace jtype

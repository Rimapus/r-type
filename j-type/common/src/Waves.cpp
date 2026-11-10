/*
** EPITECH PROJECT, 2026
** j-type/common/src/Waves
** File description:
** Waves source file
*/

#include "jtype/common/Waves.hpp"

#include <cmath>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string_view>
#include <vector>

namespace jtype {

namespace {

/**
 * @brief Read the name of a type of Bydo, as in the file names of its sprites.
 *
 * @param name The name, e.g. "sentry_drone"
 *
 * @return The type, or std::nullopt for an unknown name.
 */
std::optional<EnemyType> parseType(std::string_view name)
{
    if (name == "blobling") {
        return EnemyType::Blobling;
    }
    if (name == "jelly") {
        return EnemyType::Jelly;
    }
    if (name == "sentry_drone") {
        return EnemyType::SentryDrone;
    }
    return std::nullopt;
}

/**
 * @brief Read the name of a movement pattern.
 *
 * @param name The name, e.g. "sine_wave"
 *
 * @return The pattern, or std::nullopt for an unknown name.
 */
std::optional<MovementPattern> parsePattern(std::string_view name)
{
    if (name == "straight") {
        return MovementPattern::Straight;
    }
    if (name == "sine_wave") {
        return MovementPattern::SineWave;
    }
    if (name == "chase") {
        return MovementPattern::Chase;
    }
    return std::nullopt;
}

/**
 * @brief Read the name of a boss, as in the file names of its sprites.
 *
 * @param name The name, e.g. "incubator"
 *
 * @return The boss, or std::nullopt for an unknown name.
 */
std::optional<BossType> parseBoss(std::string_view name)
{
    if (name == "incubator") {
        return BossType::Incubator;
    }
    return std::nullopt;
}

/**
 * @brief Read the two heights of a wall or of a zapper, after its name: "<top> <bottom>".
 *
 * @param words The rest of the line
 *
 * @return The obstacle, its kind left to the caller, or why the heights are wrong.
 */
std::expected<WaveObstacle, std::string> parseObstacle(std::istringstream &words)
{
    WaveObstacle obstacle;
    std::string extra;

    if (!(words >> obstacle.top >> obstacle.bottom) || (words >> extra)) {
        return std::unexpected("an obstacle is \"wall <top> <bottom>\" or \"zapper <top> <bottom>\"");
    }
    if (!std::isfinite(obstacle.top) || !std::isfinite(obstacle.bottom) || obstacle.top < 0.0F || obstacle.bottom > 1.0F ||
        obstacle.top > obstacle.bottom) {
        return std::unexpected("the heights of an obstacle go from 0 to 1, the top one first");
    }
    return obstacle;
}

/**
 * @brief Read the name of a formation.
 *
 * @param name The name, e.g. "column"
 *
 * @return The formation, or std::nullopt for an unknown name.
 */
std::optional<Formation> parseFormation(std::string_view name)
{
    if (name == "random") {
        return Formation::Random;
    }
    if (name == "line") {
        return Formation::Line;
    }
    if (name == "column") {
        return Formation::Column;
    }
    if (name == "v") {
        return Formation::V;
    }
    return std::nullopt;
}

/**
 * @brief Read a group line: "<type> <pattern> <count> <delay> [<formation> <height>] [drop]", the height left out for
 *        random.
 *
 * @param line The line, without its end
 *
 * @return The group, or why the line is wrong.
 */
std::expected<WaveGroup, std::string> parseGroup(const std::string &line)
{
    std::istringstream words(line);
    std::string type;
    std::string pattern;
    long long count = 0;
    float delay = 0.0F;
    Formation formation = Formation::Random;
    float height = 0.5F;
    bool drop = false;
    std::vector<std::string> rest;

    if (!(words >> type >> pattern >> count >> delay)) {
        return std::unexpected("a group is \"<type> <pattern> <count> <delay> [<formation> <height>] [drop]\"");
    }
    for (std::string word; words >> word;) {
        rest.push_back(word);
    }
    if (!rest.empty() && rest.back() == "drop") {
        drop = true;
        rest.pop_back();
    }
    if (!rest.empty()) {
        const std::optional<Formation> parsedFormation = parseFormation(rest.front());

        if (!parsedFormation) {
            return std::unexpected(std::format("unknown formation \"{}\"", rest.front()));
        }
        formation = *parsedFormation;
        // A random formation has no height: each Bydo gets its own
        const std::size_t needed = formation == Formation::Random ? 1 : 2;

        if (rest.size() < needed) {
            return std::unexpected("a formation other than random needs a height");
        }
        if (rest.size() > needed) {
            return std::unexpected("a group is \"<type> <pattern> <count> <delay> [<formation> <height>] [drop]\"");
        }
        if (needed == 2) {
            std::istringstream number(rest[1]);
            std::string extra;

            if (!(number >> height) || (number >> extra)) {
                return std::unexpected("the height goes from 0 to 1");
            }
        }
    }
    const std::optional<EnemyType> parsedType = parseType(type);
    const std::optional<MovementPattern> parsedPattern = parsePattern(pattern);

    if (!parsedType) {
        return std::unexpected(std::format("unknown type \"{}\"", type));
    }
    if (!parsedPattern) {
        return std::unexpected(std::format("unknown pattern \"{}\"", pattern));
    }
    if (count <= 0 || !std::isfinite(delay) || delay < 0.0F) {
        return std::unexpected("the count must be positive and the delay not negative");
    }
    if (!std::isfinite(height) || height < 0.0F || height > 1.0F) {
        return std::unexpected("the height goes from 0 to 1");
    }
    return WaveGroup{*parsedType, *parsedPattern, static_cast<std::size_t>(count), delay, formation, height, drop};
}

} // namespace

std::expected<std::vector<Wave>, std::string> parseWaves(std::istream &input)
{
    std::vector<Wave> waves;
    std::string line;

    for (std::size_t number = 1; std::getline(input, line); ++number) {
        std::istringstream words(line);
        std::string first;

        if (!(words >> first) || first.starts_with('#')) {
            continue;
        }
        if (first == "wave") {
            float pause = 0.0F;
            std::string extra;

            if (!(words >> pause) || (words >> extra) || !std::isfinite(pause) || pause < 0.0F) {
                return std::unexpected(std::format("line {}: a wave is \"wave <pause>\", the pause not negative", number));
            }
            waves.push_back(Wave{pause, {}});
            continue;
        }
        if (waves.empty()) {
            return std::unexpected(std::format("line {}: a group, an obstacle or a boss comes after a \"wave\" line", number));
        }
        if (first == "wall" || first == "zapper") {
            auto obstacle = parseObstacle(words);

            if (!obstacle) {
                return std::unexpected(std::format("line {}: {}", number, obstacle.error()));
            }
            obstacle->kind = first == "wall" ? ObstacleKind::Block : ObstacleKind::ZapperNode;
            waves.back().obstacles.push_back(*obstacle);
            continue;
        }
        if (first == "boss") {
            std::string name;
            std::string extra;

            if (!(words >> name) || (words >> extra)) {
                return std::unexpected(std::format("line {}: a boss is \"boss <name>\"", number));
            }
            const std::optional<BossType> boss = parseBoss(name);

            if (!boss) {
                return std::unexpected(std::format("line {}: unknown boss \"{}\"", number, name));
            }
            if (waves.back().boss) {
                return std::unexpected(std::format("line {}: a wave has one boss", number));
            }
            waves.back().boss = boss;
            continue;
        }
        auto group = parseGroup(line);

        if (!group) {
            return std::unexpected(std::format("line {}: {}", number, group.error()));
        }
        waves.back().groups.push_back(*group);
    }
    if (waves.empty()) {
        return std::unexpected("no wave");
    }
    return waves;
}

std::expected<std::vector<Wave>, std::string> loadWaves(const std::filesystem::path &path)
{
    std::ifstream file(path);

    if (!file) {
        return std::unexpected(std::format("cannot open {}", path.string()));
    }
    auto waves = parseWaves(file);

    if (!waves) {
        return std::unexpected(std::format("{}: {}", path.string(), waves.error()));
    }
    return waves;
}

} // namespace jtype

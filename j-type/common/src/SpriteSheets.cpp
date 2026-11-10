/*
** EPITECH PROJECT, 2026
** j-type/common/src/SpriteSheets
** File description:
** SpriteSheets source file
*/

#include "jtype/common/SpriteSheets.hpp"

#include <cmath>
#include <cstddef>
#include <format>
#include <fstream>
#include <sstream>
#include <utility>

namespace jtype {

namespace {

/**
 * @brief Check that the numbers of a sprite can describe frames of a sheet.
 *
 * @param sheet The sprite
 *
 * @return true if the sizes, the columns and the count are positive, the first frame not negative,
 *         the duration a positive number and the origin finite.
 */
bool isValid(const SpriteSheet &sheet)
{
    return sheet.frameSize.x > 0 && sheet.frameSize.y > 0 && sheet.columns > 0 && sheet.first >= 0 && sheet.count > 0 && std::isfinite(sheet.frameDuration) && sheet.frameDuration > 0.0F && std::isfinite(sheet.origin.x) && std::isfinite(sheet.origin.y);
}

/**
 * @brief Read a sprite line, after its name.
 *
 * @param words The rest of the line
 *
 * @return The sprite, or why the line is wrong.
 */
std::expected<SpriteSheet, std::string> parseSheet(std::istringstream &words)
{
    SpriteSheet sheet;
    std::string mode;
    std::string extra;

    if (!(words >> sheet.path >> sheet.frameSize.x >> sheet.frameSize.y >> sheet.columns >> sheet.first >> sheet.count >> sheet.frameDuration >> mode >> sheet.origin.x >> sheet.origin.y >> sheet.layer) || (words >> extra)) {
        return std::unexpected("a sprite is \"<name> <path> <width> <height> <columns> <first> <count> <duration> "
                               "<loop|once> <origin x> <origin y> <layer>\"");
    }
    if (mode != "loop" && mode != "once") {
        return std::unexpected(std::format("\"{}\" is neither loop nor once", mode));
    }
    sheet.loop = mode == "loop";
    if (!isValid(sheet)) {
        return std::unexpected("the sizes, the columns, the count and the duration must be positive, the first frame "
                               "not negative");
    }
    return sheet;
}

} // namespace

std::expected<SpriteSheets, std::string> parseSpriteSheets(std::istream &input)
{
    SpriteSheets sheets;
    std::string line;

    for (std::size_t number = 1; std::getline(input, line); ++number) {
        std::istringstream words(line);
        std::string name;

        if (!(words >> name) || name.starts_with('#')) {
            continue;
        }
        auto sheet = parseSheet(words);

        if (!sheet) {
            return std::unexpected(std::format("line {}: {}", number, sheet.error()));
        }
        if (!sheets.emplace(name, std::move(*sheet)).second) {
            return std::unexpected(std::format("line {}: \"{}\" is already defined", number, name));
        }
    }
    if (sheets.empty()) {
        return std::unexpected("no sprite");
    }
    return sheets;
}

std::expected<SpriteSheets, std::string> loadSpriteSheets(const std::filesystem::path &path)
{
    std::ifstream file(path);

    if (!file) {
        return std::unexpected(std::format("cannot open {}", path.string()));
    }
    auto sheets = parseSpriteSheets(file);

    if (!sheets) {
        return std::unexpected(std::format("{}: {}", path.string(), sheets.error()));
    }
    return sheets;
}

std::string_view getSpriteName(EntityType type)
{
    switch (type) {
    case EntityType::Player1:
        return "player_p1";
    case EntityType::Player2:
        return "player_p2";
    case EntityType::Player3:
        return "player_p3";
    case EntityType::Player4:
        return "player_p4";
    case EntityType::Missile1:
        return "plasma_shot_p1";
    case EntityType::Missile2:
        return "plasma_shot_p2";
    case EntityType::Missile3:
        return "plasma_shot_p3";
    case EntityType::Missile4:
        return "plasma_shot_p4";
    case EntityType::EnemyMissile:
        return "enemy_orb";
    case EntityType::Jelly:
        return "jelly";
    case EntityType::SentryDrone:
        return "sentry_drone";
    case EntityType::Explosion:
        return "explosion_small";
    case EntityType::IncubatorBody:
        return "incubator_body";
    case EntityType::IncubatorEye:
        return "incubator_eye";
    case EntityType::IncubatorEyeAngry:
        return "incubator_eye_angry";
    case EntityType::IncubatorOrb:
        return "incubator_orb";
    case EntityType::IncubatorSpit:
        return "incubator_spit";
    case EntityType::IncubatorWreck:
        return "incubator_body_death";
    case EntityType::SpreadPellet1:
        return "spread_pellet_p1";
    case EntityType::SpreadPellet2:
        return "spread_pellet_p2";
    case EntityType::SpreadPellet3:
        return "spread_pellet_p3";
    case EntityType::SpreadPellet4:
        return "spread_pellet_p4";
    case EntityType::ExtraLifePowerUp:
        return "extra_life";
    case EntityType::ShieldPowerUp:
        return "shield";
    case EntityType::SpreadShotPowerUp:
        return "weapon_spread";
    case EntityType::Block:
        return "block";
    case EntityType::CrackedBlock:
        return "block_cracked";
    case EntityType::ZapperNode:
        return "zapper";
    case EntityType::ZapperBeam:
        return "zapper_beam_v";
    case EntityType::Blobling:
        break;
    }
    return "blobling";
}

} // namespace jtype

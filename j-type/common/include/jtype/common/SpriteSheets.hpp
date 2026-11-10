/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/SpriteSheets
** File description:
** SpriteSheets header file
*/

#pragma once

#include <expected>
#include <filesystem>
#include <istream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "jtype/common/EntityType.hpp"
#include "younity/core/Vector2.hpp"

namespace jtype {

/**
 * @brief How a sprite is drawn: an animation of a sheet of frames of the same size,
 *        numbered from 0, left to right then top to bottom, as the GridLayout of Younity.
 */
struct SpriteSheet
{
    // The image of the sheet, relative to the assets folder
    std::string path;
    // The size of one frame, in pixels
    younity::Vector2i frameSize;
    // The number of frames on a row of the sheet
    int columns = 1;
    // The number of the first frame of the animation
    int first = 0;
    // The number of frames of the animation
    int count = 1;
    // How long each frame is shown, in seconds
    float frameDuration = 0.1F;
    // true to start again after the last frame, false to stop on it
    bool loop = true;
    // The point of the frame placed on the position of the entity, in pixels
    younity::Vector2f origin;
    // The higher layers are drawn over the lower ones
    int layer = 0;

    bool operator==(const SpriteSheet &other) const = default;
};

/**
 * @brief The sprites of the game, by name.
 */
using SpriteSheets = std::unordered_map<std::string, SpriteSheet>;

/**
 * @brief Read sprites written as text, the format of assets/data/sprites.txt, one per line:
 *        "<name> <path> <width> <height> <columns> <first> <count> <duration> <loop|once> <origin x> <origin y> <layer>".
 *        Empty lines and the lines starting with # are skipped.
 *
 * @param input The text
 *
 * @return The sprites, or the line and the reason of the first error.
 */
[[nodiscard]] std::expected<SpriteSheets, std::string> parseSpriteSheets(std::istream &input);

/**
 * @brief Read the sprites of a file, see parseSpriteSheets().
 *
 * @param path The file
 *
 * @return The sprites, or why the file cannot be read.
 */
[[nodiscard]] std::expected<SpriteSheets, std::string> loadSpriteSheets(const std::filesystem::path &path);

/**
 * @brief Find the name of the sprite of an entity type, in assets/data/sprites.txt.
 *
 * @param type The type of the entity
 *
 * @return The name of its sprite.
 */
[[nodiscard]] std::string_view getSpriteName(EntityType type);

} // namespace jtype

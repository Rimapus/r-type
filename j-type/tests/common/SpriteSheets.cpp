/*
** EPITECH PROJECT, 2026
** j-type/tests/common/SpriteSheets
** File description:
** Unit tests of the sprite files
*/

#include <criterion/criterion.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include "jtype/common/EntityType.hpp"
#include "jtype/common/SpriteSheets.hpp"
#include "younity/core/Vector2.hpp"

using jtype::EntityType;
using jtype::SpriteSheet;

namespace {

auto parse(const std::string &text)
{
    std::istringstream input(text);

    return jtype::parseSpriteSheets(input);
}

const std::filesystem::path ASSETS = std::filesystem::path(JTYPE_SOURCE_DIR) / "assets";

/**
 * @brief Read the size of a PNG image from its header, without decoding it.
 */
std::optional<younity::Vector2i> readPngSize(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    std::array<unsigned char, 24> header{};

    if (!file.read(reinterpret_cast<char *>(header.data()), header.size())) {
        return std::nullopt;
    }
    // The width and the height follow the signature and the IHDR chunk header, big-endian
    auto read = [&header](std::size_t at) {
        return static_cast<int>((std::uint32_t{header[at]} << 24) | (std::uint32_t{header[at + 1]} << 16) | (std::uint32_t{header[at + 2]} << 8) | std::uint32_t{header[at + 3]});
    };
    return younity::Vector2i{read(16), read(20)};
}

} // namespace

Test(common_sprite_sheets, reads_the_sprites_by_name)
{
    const auto sheets = parse("# a comment\n"
                              "\n"
                              "ship sprites/ship.png 40 36 6 0 6 0.1 loop 24 19 2\n"
                              "boom sprites/boom.png 24 24 7 2 5 0.05 once 12 12 3\n");

    cr_assert(sheets.has_value());
    cr_assert_eq(sheets->size(), 2);
    const SpriteSheet ship{
        .path = "sprites/ship.png",
        .frameSize = {40,    36   },
        .columns = 6,
        .first = 0,
        .count = 6,
        .frameDuration = 0.1F,
        .loop = true,
        .origin = {24.0F, 19.0F},
        .layer = 2
    };
    const SpriteSheet boom{
        .path = "sprites/boom.png",
        .frameSize = {24,    24   },
        .columns = 7,
        .first = 2,
        .count = 5,
        .frameDuration = 0.05F,
        .loop = false,
        .origin = {12.0F, 12.0F},
        .layer = 3
    };

    cr_assert(sheets->at("ship") == ship);
    cr_assert(sheets->at("boom") == boom);
}

Test(common_sprite_sheets, tells_the_line_of_an_error)
{
    const auto sheets = parse("ship sprites/ship.png 40 36 6 0 6 0.1 loop 24 19 2\n"
                              "boom sprites/boom.png 24 24 7 0 7 0.05 twice 12 12 3\n");

    cr_assert_not(sheets.has_value());
    cr_assert(sheets.error().starts_with("line 2:"));
}

Test(common_sprite_sheets, rejects_wrong_lines)
{
    cr_assert_not(parse("ship sprites/ship.png 40 36 6 0 6 0.1 loop 24 19\n").has_value());
    cr_assert_not(parse("ship sprites/ship.png 40 36 6 0 6 0.1 loop 24 19 2 extra\n").has_value());
    cr_assert_not(parse("ship sprites/ship.png 0 36 6 0 6 0.1 loop 24 19 2\n").has_value());
    cr_assert_not(parse("ship sprites/ship.png 40 36 0 0 6 0.1 loop 24 19 2\n").has_value());
    cr_assert_not(parse("ship sprites/ship.png 40 36 6 -1 6 0.1 loop 24 19 2\n").has_value());
    cr_assert_not(parse("ship sprites/ship.png 40 36 6 0 0 0.1 loop 24 19 2\n").has_value());
    cr_assert_not(parse("ship sprites/ship.png 40 36 6 0 6 0 loop 24 19 2\n").has_value());
    cr_assert_not(parse("# nothing\n").has_value());
}

Test(common_sprite_sheets, rejects_a_name_defined_twice)
{
    const auto sheets = parse("ship sprites/ship.png 40 36 6 0 6 0.1 loop 24 19 2\n"
                              "ship sprites/other.png 40 36 6 0 6 0.1 loop 24 19 2\n");

    cr_assert_not(sheets.has_value());
    cr_assert(sheets.error().starts_with("line 2: \"ship\" is already defined"));
}

Test(common_sprite_sheets, missing_file_is_an_error)
{
    cr_assert_not(jtype::loadSpriteSheets("missing/sprites.txt").has_value());
}

Test(common_sprite_sheets, every_entity_type_has_a_sprite)
{
    const auto sheets = jtype::loadSpriteSheets(ASSETS / "data" / "sprites.txt");

    cr_assert(sheets.has_value(), "%s", sheets ? "" : sheets.error().c_str());
    // Explosion is the last type: a new one is added after it
    for (int type = 0; type <= static_cast<int>(EntityType::Explosion); ++type) {
        const std::string name(jtype::getSpriteName(static_cast<EntityType>(type)));

        cr_assert(sheets->contains(name), "no sprite \"%s\"", name.c_str());
    }
}

Test(common_sprite_sheets, every_animation_fits_in_its_image)
{
    const auto sheets = jtype::loadSpriteSheets(ASSETS / "data" / "sprites.txt");

    cr_assert(sheets.has_value(), "%s", sheets ? "" : sheets.error().c_str());
    for (const auto &[name, sheet] : *sheets) {
        const std::optional<younity::Vector2i> size = readPngSize(ASSETS / sheet.path);
        const int last = sheet.first + sheet.count - 1;

        cr_assert(size.has_value(), "%s: cannot read %s", name.c_str(), sheet.path.c_str());
        cr_assert_leq(sheet.columns * sheet.frameSize.x, size->x, "%s: the columns are wider than the image", name.c_str());
        cr_assert_leq((last / sheet.columns + 1) * sheet.frameSize.y, size->y, "%s: the frames go below the image",
                      name.c_str());
    }
}

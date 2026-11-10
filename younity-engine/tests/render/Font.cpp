/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/Font
** File description:
** Font unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <utility>

#include "younity/assets/ResourceManager.hpp"
#include "younity/render/Font.hpp"

using younity::assets::AssetError;
using younity::render::Font;

namespace {

// Tuffy, by Thatcher Ulrich, is in the public domain. Opening a font needs no graphics context
const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "render" / "fixtures";

} // namespace

Test(render_font, loads_a_font)
{
    const auto font = Font::loadFromFile(FIXTURES / "tuffy.ttf");

    cr_assert(font.has_value());
    cr_assert_not_null(font->getImpl());
}

Test(render_font, missing_file_is_not_found)
{
    const auto font = Font::loadFromFile(FIXTURES / "missing.ttf");

    cr_assert_not(font.has_value());
    cr_assert_eq(font.error(), AssetError::NotFound);
}

Test(render_font, folder_is_not_found)
{
    const auto font = Font::loadFromFile(FIXTURES);

    cr_assert_not(font.has_value());
    cr_assert_eq(font.error(), AssetError::NotFound);
}

Test(render_font, file_that_is_not_a_font_fails_to_load)
{
    const auto font = Font::loadFromFile(FIXTURES / "not_an_image.png");

    cr_assert_not(font.has_value());
    cr_assert_eq(font.error(), AssetError::LoadFailed);
}

Test(render_font, move_keeps_the_font)
{
    auto loaded = Font::loadFromFile(FIXTURES / "tuffy.ttf");

    cr_assert(loaded.has_value());
    const Font::Impl *impl = loaded->getImpl();
    const Font moved(std::move(*loaded));
    cr_assert_eq(moved.getImpl(), impl);
    // NOLINTNEXTLINE(bugprone-use-after-move): the moved-from state is what is tested
    cr_assert_null(loaded->getImpl());
}

Test(render_font, move_assignment_replaces_the_font)
{
    auto first = Font::loadFromFile(FIXTURES / "tuffy.ttf");
    auto second = Font::loadFromFile(FIXTURES / "tuffy.ttf");

    cr_assert(first.has_value() && second.has_value());
    const Font::Impl *impl = second->getImpl();
    *first = std::move(*second);
    cr_assert_eq(first->getImpl(), impl);
    // NOLINTNEXTLINE(bugprone-use-after-move): the moved-from state is what is tested
    cr_assert_null(second->getImpl());
}

Test(render_font, resource_manager_loads_and_shares_it)
{
    younity::assets::ResourceManager resources(FIXTURES);

    resources.setLoader<Font>(Font::loadFromFile);
    const auto first = resources.load<Font>("tuffy.ttf");
    const auto second = resources.load<Font>("tuffy.ttf");
    cr_assert(first.has_value() && second.has_value());
    cr_assert_eq(first->get(), second->get());
}

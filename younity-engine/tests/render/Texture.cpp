/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/Texture
** File description:
** Texture unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <utility>

#include "../TestSupport.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/render/Texture.hpp"

using younity::assets::AssetError;
using younity::render::Texture;
using younity::test::skipWithoutDisplay;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "render" / "fixtures";

} // namespace

Test(render_texture, missing_file_is_not_found)
{
    const auto texture = Texture::loadFromFile("does/not/exist.png");

    cr_assert_not(texture.has_value());
    cr_assert_eq(texture.error(), AssetError::NotFound);
}

Test(render_texture, is_a_loader_of_the_resource_manager)
{
    younity::assets::ResourceManager resources(std::filesystem::temp_directory_path());

    resources.setLoader<Texture>(Texture::loadFromFile);
    const auto texture = resources.load<Texture>("younity_render_missing_texture.png");
    cr_assert_not(texture.has_value());
    cr_assert_eq(texture.error(), AssetError::NotFound);
}

Test(render_texture, loads_an_image)
{
    skipWithoutDisplay();
    const auto texture = Texture::loadFromFile(FIXTURES / "red_3x2.png");

    cr_assert(texture.has_value());
    cr_assert_not_null(texture->getImpl());
}

Test(render_texture, size_is_the_one_of_the_image)
{
    skipWithoutDisplay();
    const auto texture = Texture::loadFromFile(FIXTURES / "red_3x2.png");

    cr_assert(texture.has_value());
    cr_assert_eq(texture->getSize().x, 3U);
    cr_assert_eq(texture->getSize().y, 2U);
}

Test(render_texture, file_that_is_not_an_image_fails_to_load)
{
    skipWithoutDisplay();
    const auto texture = Texture::loadFromFile(FIXTURES / "not_an_image.png");

    cr_assert_not(texture.has_value());
    cr_assert_eq(texture.error(), AssetError::LoadFailed);
}

Test(render_texture, folder_is_not_found)
{
    const auto texture = Texture::loadFromFile(FIXTURES);

    cr_assert_not(texture.has_value());
    cr_assert_eq(texture.error(), AssetError::NotFound);
}

Test(render_texture, move_keeps_the_image)
{
    skipWithoutDisplay();
    auto loaded = Texture::loadFromFile(FIXTURES / "red_3x2.png");

    cr_assert(loaded.has_value());
    const Texture moved(std::move(*loaded));
    cr_assert_eq(moved.getSize().x, 3U);
    cr_assert_not_null(moved.getImpl());
}

Test(render_texture, moved_from_texture_is_empty)
{
    skipWithoutDisplay();
    auto loaded = Texture::loadFromFile(FIXTURES / "red_3x2.png");

    cr_assert(loaded.has_value());
    const Texture moved(std::move(*loaded));
    // NOLINTNEXTLINE(bugprone-use-after-move): the moved-from state is what is tested
    cr_assert_null(loaded->getImpl());
    cr_assert_eq(loaded->getSize().x, 0U);
    cr_assert_eq(loaded->getSize().y, 0U);
}

Test(render_texture, move_assignment_replaces_the_image)
{
    skipWithoutDisplay();
    auto first = Texture::loadFromFile(FIXTURES / "red_3x2.png");
    auto second = Texture::loadFromFile(FIXTURES / "red_3x2.png");

    cr_assert(first.has_value() && second.has_value());
    const Texture::Impl *image = second->getImpl();
    *first = std::move(*second);
    cr_assert_eq(first->getImpl(), image);
    // NOLINTNEXTLINE(bugprone-use-after-move): the moved-from state is what is tested
    cr_assert_null(second->getImpl());
}

Test(render_texture, resource_manager_loads_and_shares_it)
{
    skipWithoutDisplay();
    younity::assets::ResourceManager resources(FIXTURES);

    resources.setLoader<Texture>(Texture::loadFromFile);
    const auto first = resources.load<Texture>("red_3x2.png");
    const auto second = resources.load<Texture>("red_3x2.png");
    cr_assert(first.has_value() && second.has_value());
    cr_assert_eq(first->get(), second->get());
}

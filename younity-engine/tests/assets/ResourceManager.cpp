/*
** EPITECH PROJECT, 2026
** younity-engine/tests/assets/ResourceManager
** File description:
** ResourceManager unit tests
*/

#include <criterion/criterion.h>

#include <expected>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

#include "younity/assets/ResourceManager.hpp"

using younity::assets::AssetError;
using younity::assets::Loader;
using younity::assets::ResourceManager;

namespace {

// Two types of resource, to check that each one has its own cache
struct FakeTexture
{
    std::string path;
};

struct FakeSound
{
    std::string path;
};

template <typename T>
Loader<T> makeLoader(int &loads)
{
    return [&loads](const std::filesystem::path &path) -> std::expected<T, AssetError> {
        ++loads;
        return T{path.string()};
    };
}

} // namespace

Test(assets_resource_manager, load_uses_the_loader_of_the_type)
{
    int textureLoads = 0;
    int soundLoads = 0;
    ResourceManager resources;

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(textureLoads));
    resources.setLoader<FakeSound>(makeLoader<FakeSound>(soundLoads));
    cr_assert((*resources.load<FakeTexture>("ship.png"))->path == "ship.png");
    cr_assert((*resources.load<FakeSound>("shot.wav"))->path == "shot.wav");
    cr_assert_eq(textureLoads, 1);
    cr_assert_eq(soundLoads, 1);
}

Test(assets_resource_manager, second_load_comes_from_the_cache)
{
    int loads = 0;
    ResourceManager resources;

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(loads));

    const auto first = resources.load<FakeTexture>("ship.png");
    const auto second = resources.load<FakeTexture>("./ship.png");

    cr_assert_eq(loads, 1);
    cr_assert_eq(first->get(), second->get());
}

Test(assets_resource_manager, type_without_loader_gives_no_loader)
{
    ResourceManager resources;

    cr_assert_not(resources.hasLoader<FakeTexture>());
    cr_assert_eq(resources.load<FakeTexture>("ship.png").error(), AssetError::NoLoader);
    cr_assert_not(resources.contains<FakeTexture>("ship.png"));
    resources.unload<FakeTexture>("ship.png");
}

Test(assets_resource_manager, same_path_is_cached_once_per_type)
{
    int textureLoads = 0;
    int soundLoads = 0;
    ResourceManager resources;

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(textureLoads));
    resources.setLoader<FakeSound>(makeLoader<FakeSound>(soundLoads));
    (void)resources.load<FakeTexture>("boss");
    cr_assert(resources.contains<FakeTexture>("boss"));
    cr_assert_not(resources.contains<FakeSound>("boss"));
    (void)resources.load<FakeSound>("boss");
    cr_assert_eq(soundLoads, 1);
}

Test(assets_resource_manager, loader_receives_the_path_inside_the_asset_folder)
{
    int loads = 0;
    ResourceManager resources("assets");

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(loads));
    cr_assert((*resources.load<FakeTexture>("ship.png"))->path == (std::filesystem::path("assets") / "ship.png").string());
}

Test(assets_resource_manager, find_path_gives_the_file_of_a_resource)
{
    int loads = 0;
    ResourceManager resources;

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(loads));

    const auto ship = resources.load<FakeTexture>("ship.png");

    cr_assert(resources.findPath(*ship) == std::filesystem::path("ship.png"));
    cr_assert_not(resources.findPath(std::make_shared<const FakeSound>()).has_value());
}

Test(assets_resource_manager, unload_removes_only_its_type)
{
    int textureLoads = 0;
    int soundLoads = 0;
    ResourceManager resources;

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(textureLoads));
    resources.setLoader<FakeSound>(makeLoader<FakeSound>(soundLoads));
    (void)resources.load<FakeTexture>("boss");
    (void)resources.load<FakeSound>("boss");
    resources.unload<FakeTexture>("boss");
    cr_assert_not(resources.contains<FakeTexture>("boss"));
    cr_assert(resources.contains<FakeSound>("boss"));
}

Test(assets_resource_manager, clear_empties_every_cache_and_keeps_the_loaders)
{
    int textureLoads = 0;
    int soundLoads = 0;
    ResourceManager resources;

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(textureLoads));
    resources.setLoader<FakeSound>(makeLoader<FakeSound>(soundLoads));
    (void)resources.load<FakeTexture>("ship.png");
    (void)resources.load<FakeSound>("shot.wav");
    resources.clear();
    cr_assert_not(resources.contains<FakeTexture>("ship.png"));
    cr_assert_not(resources.contains<FakeSound>("shot.wav"));
    cr_assert(resources.hasLoader<FakeTexture>());
    cr_assert(resources.load<FakeTexture>("ship.png").has_value());
    cr_assert_eq(textureLoads, 2);
}

Test(assets_resource_manager, new_loader_drops_the_resources_of_the_old_one)
{
    int oldLoads = 0;
    int newLoads = 0;
    ResourceManager resources;

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(oldLoads));
    const std::shared_ptr<const FakeTexture> kept = *resources.load<FakeTexture>("ship.png");

    resources.setLoader<FakeTexture>(makeLoader<FakeTexture>(newLoads));
    cr_assert_not(resources.contains<FakeTexture>("ship.png"));
    (void)resources.load<FakeTexture>("ship.png");
    cr_assert_eq(newLoads, 1);
    cr_assert(kept->path == "ship.png");
}

Test(assets_resource_manager, empty_loader_is_refused)
{
    ResourceManager resources;

    cr_assert_throw(resources.setLoader<FakeTexture>(Loader<FakeTexture>{}), std::invalid_argument);
    cr_assert_not(resources.hasLoader<FakeTexture>());
}

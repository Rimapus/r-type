/*
** EPITECH PROJECT, 2026
** younity-engine/tests/assets/ResourceCache
** File description:
** ResourceCache unit tests
*/

#include <criterion/criterion.h>

#include <expected>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

#include "younity/assets/ResourceCache.hpp"

using younity::assets::AssetError;
using younity::assets::Loader;
using younity::assets::ResourceCache;

namespace {

// Stands for a texture: remembers the file it was loaded from
struct FakeTexture
{
    std::string path;
};

// Counts its calls, and fails on the files the test marks as missing
struct FakeDisk
{
    int loads = 0;
    std::string missing;
};

Loader<FakeTexture> makeLoader(FakeDisk &disk)
{
    return [&disk](const std::filesystem::path &path) -> std::expected<FakeTexture, AssetError> {
        ++disk.loads;
        if (path == disk.missing) {
            return std::unexpected(AssetError::NotFound);
        }
        return FakeTexture{path.string()};
    };
}

} // namespace

Test(assets_resource_cache, load_gives_the_resource_of_the_loader)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    const auto texture = cache.load("ship.png");

    cr_assert(texture.has_value());
    cr_assert_eq((*texture)->path, std::string("ship.png"));
}

Test(assets_resource_cache, second_load_comes_from_the_cache)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    const auto first = cache.load("ship.png");
    const auto second = cache.load("ship.png");

    cr_assert_eq(disk.loads, 1);
    cr_assert_eq(first->get(), second->get());
}

Test(assets_resource_cache, different_files_are_loaded_once_each)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    (void)cache.load("ship.png");
    (void)cache.load("missile.png");
    (void)cache.load("ship.png");
    cr_assert_eq(disk.loads, 2);
    cr_assert_eq(cache.getSize(), std::size_t{2});
}

Test(assets_resource_cache, failure_is_returned_and_not_cached)
{
    FakeDisk disk{.loads = 0, .missing = "boss.png"};
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    const auto first = cache.load("boss.png");
    const auto second = cache.load("boss.png");

    cr_assert_eq(first.error(), AssetError::NotFound);
    cr_assert_eq(second.error(), AssetError::NotFound);
    cr_assert_eq(disk.loads, 2);
    cr_assert_not(cache.contains("boss.png"));
}

Test(assets_resource_cache, failed_file_loads_once_it_exists)
{
    FakeDisk disk{.loads = 0, .missing = "boss.png"};
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    (void)cache.load("boss.png");
    disk.missing.clear();
    cr_assert(cache.load("boss.png").has_value());
    cr_assert(cache.contains("boss.png"));
}

Test(assets_resource_cache, contains_only_loaded_files)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    cr_assert_not(cache.contains("ship.png"));
    (void)cache.load("ship.png");
    cr_assert(cache.contains("ship.png"));
}

Test(assets_resource_cache, find_path_gives_the_file_of_a_resource)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));
    const auto ship = cache.load("sprites/../sprites/ship.png");
    const auto unknown = std::make_shared<const FakeTexture>();

    cr_assert(cache.findPath(*ship) == std::filesystem::path("sprites/ship.png"));
    cr_assert_not(cache.findPath(unknown).has_value());
    cr_assert_not(cache.findPath(nullptr).has_value());
    cache.unload("sprites/ship.png");
    cr_assert_not(cache.findPath(*ship).has_value());
}

Test(assets_resource_cache, unload_makes_the_next_load_read_the_file_again)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    (void)cache.load("ship.png");
    cache.unload("ship.png");
    cr_assert_not(cache.contains("ship.png"));
    (void)cache.load("ship.png");
    cr_assert_eq(disk.loads, 2);
}

Test(assets_resource_cache, unload_of_an_unknown_file_does_nothing)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    (void)cache.load("ship.png");
    cache.unload("missile.png");
    cr_assert_eq(cache.getSize(), std::size_t{1});
}

Test(assets_resource_cache, resource_kept_by_the_game_survives_unload_and_clear)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    const std::shared_ptr<const FakeTexture> ship = *cache.load("ship.png");

    cache.unload("ship.png");
    cache.clear();
    cr_assert_eq(ship->path, std::string("ship.png"));
}

Test(assets_resource_cache, clear_empties_the_cache)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    (void)cache.load("ship.png");
    (void)cache.load("missile.png");
    cache.clear();
    cr_assert_eq(cache.getSize(), std::size_t{0});
    cr_assert_not(cache.contains("ship.png"));
}

Test(assets_resource_cache, empty_loader_is_refused)
{
    cr_assert_throw(ResourceCache<FakeTexture>(Loader<FakeTexture>{}), std::invalid_argument);
}

Test(assets_resource_cache, loader_receives_the_path_inside_the_asset_folder)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk), "assets");

    cr_assert_eq((*cache.load("sprites/ship.png"))->path, (std::filesystem::path("assets") / "sprites/ship.png").string());
}

Test(assets_resource_cache, ways_to_write_the_same_file_share_one_resource)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk), "assets");

    const auto first = cache.load("sprites/ship.png");
    const auto second = cache.load("./sprites/enemies/../ship.png");

    cr_assert_eq(disk.loads, 1);
    cr_assert_eq(first->get(), second->get());
    cr_assert(cache.contains("sprites//ship.png"));
}

Test(assets_resource_cache, path_going_out_of_the_folder_is_refused_without_loading)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk), "assets");

    cr_assert_eq(cache.load("../save.dat").error(), AssetError::InvalidPath);
    cr_assert_eq(cache.load("/etc/passwd").error(), AssetError::InvalidPath);
    cr_assert_eq(disk.loads, 0);
    cr_assert_not(cache.contains("../save.dat"));
}

Test(assets_resource_cache, unload_accepts_any_way_to_write_the_file)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    (void)cache.load("sprites/ship.png");
    cache.unload("./sprites/ship.png");
    cache.unload("../outside.png");
    cr_assert_eq(cache.getSize(), std::size_t{0});
}

Test(assets_resource_cache, failure_does_not_grow_the_cache)
{
    FakeDisk disk{.loads = 0, .missing = "boss.png"};
    ResourceCache<FakeTexture> cache(makeLoader(disk));

    (void)cache.load("ship.png");
    (void)cache.load("boss.png");
    cr_assert_eq(cache.getSize(), std::size_t{1});
}

Test(assets_resource_cache, exception_of_the_loader_goes_through_and_caches_nothing)
{
    ResourceCache<FakeTexture> cache([](const std::filesystem::path &) -> std::expected<FakeTexture, AssetError> {
        throw std::runtime_error("corrupted file");
    });

    cr_assert_throw((void)cache.load("ship.png"), std::runtime_error);
    cr_assert_eq(cache.getSize(), std::size_t{0});
}

Test(assets_resource_cache, loader_can_load_another_file_of_the_same_cache)
{
    ResourceCache<FakeTexture> *self = nullptr;
    ResourceCache<FakeTexture> cache([&self](const std::filesystem::path &path) -> std::expected<FakeTexture, AssetError> {
        // A sheet of sprites loads the picture it is cut from
        if (path == "ship.sheet") {
            return FakeTexture{(*self->load("ship.png"))->path};
        }
        return FakeTexture{path.string()};
    });

    self = &cache;
    cr_assert((*cache.load("ship.sheet"))->path == "ship.png");
    cr_assert_eq(cache.getSize(), std::size_t{2});
}

Test(assets_resource_cache, file_dropped_by_everyone_is_read_again_after_unload)
{
    FakeDisk disk;
    ResourceCache<FakeTexture> cache(makeLoader(disk));
    std::weak_ptr<const FakeTexture> watched;

    {
        const std::shared_ptr<const FakeTexture> ship = *cache.load("ship.png");

        watched = ship;
        cache.unload("ship.png");
    }
    cr_assert(watched.expired());
    (void)cache.load("ship.png");
    cr_assert_eq(disk.loads, 2);
}

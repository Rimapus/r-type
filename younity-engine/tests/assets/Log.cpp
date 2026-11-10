/*
** EPITECH PROJECT, 2026
** younity-engine/tests/assets/Log
** File description:
** Messages of the assets module unit tests
*/

#include <criterion/criterion.h>

#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include "younity/assets/Log.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/logger/Global.hpp"
#include "younity/logger/MemorySink.hpp"

using younity::assets::AssetError;
using younity::assets::ResourceManager;
using younity::logger::Level;
using younity::logger::MemorySink;

namespace {

struct FakeTexture
{
    std::string path;
};

// Keeps the messages of the test, read through the returned reference.
// The sink is added to the global registry: it stays for the test only because Criterion runs each test in its own process
const MemorySink &listen(void)
{
    auto sink = std::make_unique<MemorySink>();
    const MemorySink &memory = *sink;

    younity::logger::registry().addSink("memory", std::move(sink));
    return memory;
}

ResourceManager makeResources(void)
{
    ResourceManager resources;

    resources.setLoader<FakeTexture>([](const std::filesystem::path &path) -> std::expected<FakeTexture, AssetError> {
        if (path == "missing.png") {
            return std::unexpected(AssetError::NotFound);
        }
        return FakeTexture{path.string()};
    });
    return resources;
}

} // namespace

Test(assets_log, logger_is_named_after_the_module)
{
    cr_assert(younity::assets::logger().getName() == "assets");
}

Test(assets_log, load_is_reported)
{
    const MemorySink &memory = listen();
    ResourceManager resources = makeResources();

    (void)resources.load<FakeTexture>("ship.png");
    cr_assert_eq(memory.getRecords().size(), 1);
    cr_assert_eq(memory.getRecords().front().getLevel(), Level::Info);
    cr_assert(memory.getRecords().front().getMessage() == "Loaded ship.png");
}

Test(assets_log, failure_is_a_warning_with_its_reason)
{
    const MemorySink &memory = listen();
    ResourceManager resources = makeResources();

    (void)resources.load<FakeTexture>("missing.png");
    (void)resources.load<FakeTexture>("../save.dat");
    cr_assert_eq(memory.getRecords().size(), 2);
    cr_assert_eq(memory.getRecords()[0].getLevel(), Level::Warn);
    cr_assert(memory.getRecords()[0].getMessage() == "Cannot load missing.png: not found");
    cr_assert(memory.getRecords()[1].getMessage() == "Cannot load ../save.dat: invalid path");
}

Test(assets_log, type_without_loader_is_a_warning)
{
    const MemorySink &memory = listen();
    ResourceManager resources;

    (void)resources.load<FakeTexture>("ship.png");
    cr_assert_eq(memory.getRecords().size(), 1);
    cr_assert(memory.getRecords().front().getMessage() == "Cannot load ship.png: no loader");
}

Test(assets_log, cache_hit_is_a_debug_message)
{
    const MemorySink &memory = listen();
    ResourceManager resources = makeResources();

    younity::assets::logger().setLevel(Level::Debug);
    (void)resources.load<FakeTexture>("ship.png");
    (void)resources.load<FakeTexture>("ship.png");
    cr_assert_eq(memory.getRecords().size(), 2);
    cr_assert_eq(memory.getRecords()[1].getLevel(), Level::Debug);
    cr_assert(memory.getRecords()[1].getMessage() == "ship.png comes from the cache");
}

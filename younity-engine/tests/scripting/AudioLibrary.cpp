/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/AudioLibrary
** File description:
** AudioLibrary unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <format>
#include <string>
#include <vector>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/scripting/Host.hpp"
#include "younity/scripting/components/Script.hpp"
#include "younity/scripting/systems/ScriptSystem.hpp"

using younity::EventBus;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::scripting::Host;
using younity::scripting::Script;
using younity::scripting::ScriptSystem;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "scripting" / "fixtures";

// Runs a script with what the game gives, then tells if it still runs
bool runs(const Host &host, const std::string &fixture)
{
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, host, FIXTURES);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{fixture});
    registry.updateSystems(0.5F);
    return scripts.isRunning(entity);
}

// Writes each call of the scripts to the audio of the game
Host recordingHost(std::vector<std::string> &calls)
{
    Host host{};

    host.audio.play = [&calls](const std::string &path, float volume) {
        calls.push_back(std::format("play {} {}", path, volume));
    };
    host.audio.playMusic = [&calls](const std::string &path, bool looping) {
        calls.push_back(std::format("music {} {}", path, looping ? "looping" : "once"));
    };
    host.audio.pauseMusic = [&calls] { calls.emplace_back("pause"); };
    host.audio.stopMusic = [&calls] { calls.emplace_back("stop"); };
    host.audio.setVolume = [&calls](float volume) { calls.push_back(std::format("volume {}", volume)); };
    host.audio.setMusicVolume = [&calls](float volume) { calls.push_back(std::format("music volume {}", volume)); };
    return host;
}

} // namespace

Test(scripting_audio_library, plays_sounds_and_music)
{
    std::vector<std::string> calls;

    cr_assert(runs(recordingHost(calls), "plays_audio.lua"));
    cr_assert(calls == (std::vector<std::string>{
                           "play sounds/jump.wav 0.5",
                           "play sounds/dash.wav 1",
                           "music music/theme.ogg looping",
                           "music music/end.ogg once",
                           "pause",
                           "stop",
                           "volume 0.25",
                           "music volume 1",
                       }));
}

Test(scripting_audio_library, volume_above_one_stops_the_script)
{
    std::vector<std::string> calls;

    cr_assert_not(runs(recordingHost(calls), "plays_too_loud.lua"));
    cr_assert(calls.empty());
}

Test(scripting_audio_library, sound_outside_the_assets_stops_the_script)
{
    std::vector<std::string> calls;

    cr_assert_not(runs(recordingHost(calls), "plays_outside_the_assets.lua"));
    cr_assert(calls.empty());
}

Test(scripting_audio_library, nothing_plays_without_audio)
{
    cr_assert(runs(Host{}, "plays_audio.lua"));
}

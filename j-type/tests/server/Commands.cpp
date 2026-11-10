/*
** EPITECH PROJECT, 2026
** j-type/tests/server/Commands
** File description:
** Commands unit tests
*/

#include <criterion/criterion.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Commands.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/tools/Console.hpp"

using jtype::PlayerInfo;
using jtype::ServerControl;
using jtype::SlotState;
using jtype::WaveProgress;
using younity::tools::Console;

namespace {

// A server without a socket: it keeps what the commands asked
class FakeServer : public ServerControl
{
public:
    std::vector<PlayerInfo> players{
        PlayerInfo{0, SlotState::Playing, younity::network::Endpoint{"127.0.0.1", 4242}, 1200, 3, 4521},
        PlayerInfo{1, SlotState::Free,    std::nullopt,                                  0,    0, 0   },
        PlayerInfo{2, SlotState::Left,    std::nullopt,                                  300,  0, 0   },
        PlayerInfo{3, SlotState::Out,     std::nullopt,                                  50,   0, 0   },
    };
    WaveProgress waves{3, 8};
    std::optional<std::uint8_t> kicked;
    bool restarted = false;
    bool stopped = false;
    jtype::Difficulty difficulty = jtype::Difficulty::Normal;

    [[nodiscard]] std::vector<PlayerInfo> getPlayers(void) const override
    {
        return this->players;
    }

    bool kickPlayer(std::uint8_t slot) override
    {
        if (this->players[slot].state != SlotState::Playing) {
            return false;
        }
        this->kicked = slot;
        return true;
    }

    void restartGame(void) override
    {
        this->restarted = true;
    }

    [[nodiscard]] WaveProgress getWaves(void) const override
    {
        return this->waves;
    }

    void stopServer(void) override
    {
        this->stopped = true;
    }

    [[nodiscard]] jtype::Difficulty getDifficulty(void) const override
    {
        return this->difficulty;
    }

    void setDifficulty(jtype::Difficulty value) override
    {
        this->difficulty = value;
    }
};

std::string run(Console &console, const std::string &line)
{
    const auto answer = console.execute(line);

    cr_assert(answer.has_value(), "%s", answer ? "" : answer.error().message.c_str());
    return *answer;
}

std::string fail(Console &console, const std::string &line)
{
    const auto answer = console.execute(line);

    cr_assert_not(answer.has_value());
    return answer.error().message;
}

} // namespace

Test(server_commands, players_lists_the_slots)
{
    Console console;
    FakeServer server;

    jtype::registerCommands(console, server);
    cr_assert_eq(run(console, "players"), "P1 playing 127.0.0.1:4242, score 1200, 3 lives, last input 4521\n"
                                          "P2 free\n"
                                          "P3 left, score 300\n"
                                          "P4 out of lives, score 50");
}

Test(server_commands, kick_takes_a_player_out)
{
    Console console;
    FakeServer server;

    jtype::registerCommands(console, server);
    cr_assert_eq(run(console, "kick 1"), "P1 kicked");
    cr_assert_eq(server.kicked, std::uint8_t{0});
    cr_assert_eq(fail(console, "kick 2"), "P2 is not in the game");
    cr_assert_eq(fail(console, "kick 5"), "kick takes a player, 1 to 4");
    cr_assert_eq(fail(console, "kick"), "kick takes a player, 1 to 4");
    cr_assert_eq(fail(console, "kick one"), "kick takes a player, 1 to 4");
}

Test(server_commands, restart_and_stop_reach_the_server)
{
    Console console;
    FakeServer server;

    jtype::registerCommands(console, server);
    cr_assert_eq(run(console, "restart"), "New game");
    cr_assert(server.restarted);
    cr_assert_eq(run(console, "stop"), "Stopping");
    cr_assert(server.stopped);
}

Test(server_commands, waves_tells_the_wave_played_and_the_next_one)
{
    Console console;
    FakeServer server;

    jtype::registerCommands(console, server);
    cr_assert_eq(run(console, "waves"), "Wave 3 of 8, round 1; next: wave 4");
    server.waves = WaveProgress{8, 8};
    cr_assert_eq(run(console, "waves"), "Wave 8 of 8, round 1; next: wave 1");
    server.waves = WaveProgress{9, 8};
    cr_assert_eq(run(console, "waves"), "Wave 1 of 8, round 2; next: wave 2");
    server.waves = WaveProgress{0, 8};
    cr_assert_eq(run(console, "waves"), "No wave started yet; next: wave 1 of 8");
}

Test(server_commands, are_taken_back)
{
    Console console;
    FakeServer server;

    jtype::registerCommands(console, server);
    jtype::removeCommands(console);
    cr_assert_not(console.hasCommand("players"));
    cr_assert_not(console.hasCommand("difficulty"));
    cr_assert_not(console.hasCommand("stop"));
}

Test(server_commands, difficulty_tells_and_changes_the_difficulty)
{
    Console console;
    FakeServer server;

    jtype::registerCommands(console, server);
    cr_assert_eq(run(console, "difficulty"), "Difficulty normal");
    cr_assert_eq(run(console, "difficulty easy"), "Difficulty easy: the shots now, the lives at the next game");
    cr_assert(server.difficulty == jtype::Difficulty::Easy);
    cr_assert_eq(run(console, "difficulty"), "Difficulty easy");
    cr_assert_eq(fail(console, "difficulty extreme"), "difficulty takes easy, normal or hard");
    cr_assert_eq(fail(console, "difficulty easy hard"), "difficulty takes easy, normal or hard");
    cr_assert(server.difficulty == jtype::Difficulty::Easy);
}

/*
** EPITECH PROJECT, 2026
** j-type/server/src/Commands
** File description:
** Commands source file
*/

#include "Commands.hpp"

#include <array>
#include <charconv>
#include <expected>
#include <format>
#include <span>
#include <string>
#include <string_view>

#include "jtype/common/Constants.hpp"

namespace jtype {

namespace {

using Arguments = std::span<const std::string_view>;
using Answer = std::expected<std::string, younity::tools::ConsoleError>;

// The names of the administration commands, removed together by removeCommands()
constexpr std::array<std::string_view, 6> COMMAND_NAMES{"players", "kick", "restart", "waves", "difficulty", "stop"};

/**
 * @brief Write a slot the way the players command prints it.
 *
 * @param player The slot
 *
 * @return One line, e.g. "P1 playing 127.0.0.1:4242, score 1200, 3 lives, last input 4521".
 */
std::string formatPlayer(const PlayerInfo &player)
{
    const std::string name = std::format("P{}", player.slot + 1);

    switch (player.state) {
    case SlotState::Playing:
        return std::format("{} playing {}, score {}, {} lives, last input {}", name,
                           player.address ? player.address->toString() : "?", player.score, player.lives, player.lastInput);
    case SlotState::Out:
        return std::format("{} out of lives, score {}", name, player.score);
    case SlotState::Left:
        return std::format("{} left, score {}", name, player.score);
    case SlotState::Free:
        break;
    }
    return std::format("{} free", name);
}

/**
 * @brief Read the slot of a player the way they see it, P1 to P4.
 *
 * @param text The number, from 1 to MAX_PLAYERS
 *
 * @return The slot, from 0, or nothing for another text.
 */
std::optional<std::uint8_t> parsePlayer(std::string_view text)
{
    unsigned int number = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), number);

    if (error != std::errc{} || end != text.data() + text.size() || number < 1 || number > MAX_PLAYERS) {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(number - 1);
}

/**
 * @brief Write how far the waves went, as the waves command prints it.
 *
 * @param waves The waves started and the waves of the file
 *
 * @return One line, e.g. "Wave 3 of 8, round 1; next: wave 4".
 */
std::string formatWaves(const WaveProgress &waves)
{
    if (waves.count == 0) {
        return "No wave";
    }
    if (waves.started == 0) {
        return std::format("No wave started yet; next: wave 1 of {}", waves.count);
    }
    const std::size_t played = ((waves.started - 1) % waves.count) + 1;
    const std::size_t round = ((waves.started - 1) / waves.count) + 1;

    return std::format("Wave {} of {}, round {}; next: wave {}", played, waves.count, round, (waves.started % waves.count) + 1);
}

} // namespace

void registerCommands(younity::tools::Console &console, ServerControl &control)
{
    console.addCommand("players", "List the slots: the address, the score, the lives and the last input of each player",
                       [&control](Arguments) -> Answer {
                           std::string answer;

                           for (const PlayerInfo &player : control.getPlayers()) {
                               answer += (answer.empty() ? "" : "\n") + formatPlayer(player);
                           }
                           return answer;
                       });
    console.addCommand("kick", "Take a player out of the game: kick <1-4>", [&control](Arguments arguments) -> Answer {
        const std::optional<std::uint8_t> slot = arguments.size() == 1 ? parsePlayer(arguments.front()) : std::nullopt;

        if (!slot) {
            return std::unexpected(younity::tools::ConsoleError{std::format("kick takes a player, 1 to {}", MAX_PLAYERS)});
        }
        if (!control.kickPlayer(*slot)) {
            return std::unexpected(younity::tools::ConsoleError{std::format("P{} is not in the game", *slot + 1)});
        }
        return std::format("P{} kicked", *slot + 1);
    });
    console.addCommand("restart", "Start a new game with the players in the game", [&control](Arguments) -> Answer {
        control.restartGame();
        return std::string("New game");
    });
    console.addCommand("waves", "Tell the wave played and the next one", [&control](Arguments) -> Answer {
        return formatWaves(control.getWaves());
    });
    console.addCommand("difficulty", "Tell or change the difficulty: difficulty [easy|normal|hard]",
                       [&control](Arguments arguments) -> Answer {
                           if (arguments.empty()) {
                               return std::format("Difficulty {}", getDifficultyName(control.getDifficulty()));
                           }
                           const std::optional<Difficulty> difficulty =
                               arguments.size() == 1 ? parseDifficulty(arguments.front()) : std::nullopt;

                           if (!difficulty) {
                               return std::unexpected(younity::tools::ConsoleError{"difficulty takes easy, normal or hard"});
                           }
                           control.setDifficulty(*difficulty);
                           return std::format("Difficulty {}: the shots now, the lives at the next game",
                                              getDifficultyName(*difficulty));
                       });
    console.addCommand("stop", "Stop the server", [&control](Arguments) -> Answer {
        control.stopServer();
        return std::string("Stopping");
    });
}

void removeCommands(younity::tools::Console &console)
{
    for (const std::string_view name : COMMAND_NAMES) {
        (void)console.removeCommand(name);
    }
}

} // namespace jtype

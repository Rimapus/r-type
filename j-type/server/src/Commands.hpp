/*
** EPITECH PROJECT, 2026
** j-type/server/src/Commands
** File description:
** Commands header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "jtype/common/Difficulty.hpp"
#include "jtype/common/Messages.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/tools/Console.hpp"

namespace jtype {

/**
 * @brief A slot of the game, as the players command prints it.
 */
struct PlayerInfo
{
    std::uint8_t slot = 0;
    SlotState state = SlotState::Free;
    // The address of its client, none once it left
    std::optional<younity::network::Endpoint> address;
    std::uint32_t score = 0;
    std::uint8_t lives = 0;
    // The step of the newest input of its client, 0 before the first one
    std::uint32_t lastInput = 0;
};

/**
 * @brief How far the waves went: the waves started, the ones played again included, and the waves of the file.
 */
struct WaveProgress
{
    std::size_t started = 0;
    std::size_t count = 0;
};

/**
 * @brief What the administration commands act on: the server, or a fake in the tests, which run the commands without
 *        a socket.
 */
class ServerControl
{
public:
    virtual ~ServerControl(void) = default;

    /**
     * @brief Get the four slots of the game.
     *
     * @return The slots, in order.
     */
    [[nodiscard]] virtual std::vector<PlayerInfo> getPlayers(void) const = 0;

    /**
     * @brief Take a player out of the game, like a client that left: its slot is freed, and the server ignores its
     *        client until its session times out.
     *
     * @param slot The slot, below MAX_PLAYERS
     *
     * @return false if no client plays on this slot.
     */
    virtual bool kickPlayer(std::uint8_t slot) = 0;

    /**
     * @brief Start a new game with the clients in the game.
     */
    virtual void restartGame(void) = 0;

    /**
     * @brief Get how far the waves went.
     *
     * @return The waves started and the waves of the file.
     */
    [[nodiscard]] virtual WaveProgress getWaves(void) const = 0;

    /**
     * @brief Stop the server at the end of the step.
     */
    virtual void stopServer(void) = 0;

    /**
     * @brief Get the difficulty of the game.
     *
     * @return The difficulty.
     */
    [[nodiscard]] virtual Difficulty getDifficulty(void) const = 0;

    /**
     * @brief Change the difficulty: the Bydo shots follow it at once, the lives at the next ship that joins or game
     *        that starts.
     *
     * @param difficulty The difficulty
     */
    virtual void setDifficulty(Difficulty difficulty) = 0;
};

/**
 * @brief Add the administration commands to a console: `players`, `kick <1-4>`, `restart`, `waves`,
 *        `difficulty [easy|normal|hard]` and `stop`.
 *
 * @param console The console, e.g. younity::tools::console()
 * @param control What the commands act on; it must outlive the commands, see removeCommands()
 *
 * @throw std::invalid_argument if one of the names is already used.
 */
void registerCommands(younity::tools::Console &console, ServerControl &control);

/**
 * @brief Remove the commands added by registerCommands().
 *
 * @param console The console they were added to
 */
void removeCommands(younity::tools::Console &console);

} // namespace jtype

/*
** EPITECH PROJECT, 2026
** j-type/server/src/Server
** File description:
** Server header file
*/

#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <span>
#include <string_view>
#include <vector>

#include "Commands.hpp"
#include "ConsoleInput.hpp"
#include "DropLog.hpp"
#include "EventTracker.hpp"
#include "InputGate.hpp"
#include "jtype/common/Constants.hpp"
#include "jtype/common/Difficulty.hpp"
#include "jtype/common/Messages.hpp"
#include "jtype/common/TrafficMeter.hpp"
#include "jtype/common/WaveManager.hpp"
#include "younity/core/Application.hpp"
#include "younity/core/EventBus.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/SessionTable.hpp"
#include "younity/network/UdpPump.hpp"
#include "younity/physics/CollisionEvent.hpp"

namespace jtype {

// How often the server logs its traffic: long enough to average the waves, short enough to follow a game
inline constexpr std::chrono::seconds TRAFFIC_LOG_PERIOD{10};

/**
 * @brief The game server: it owns the game, applies the inputs of the clients and sends them a snapshot at each step.
 *        It runs on the fixed-step loop of younity::Application, so it never waits on a client, and it ends cleanly
 *        once stop() is called, e.g. by the handlers of installStopHandlers() or the stop command. An administrator
 *        types the commands of its console in its terminal, see registerCommands().
 */
class Server : public younity::Application, private ServerControl
{
public:
    /**
     * @brief Listen on a port, set up the systems of the game and add the commands of the engine and of the server to
     *        the console.
     *
     * @param port The UDP port the clients connect to
     * @param difficulty The difficulty of the games, which the difficulty command changes
     *
     * @throw std::runtime_error if the port cannot be bound, or assets/data/waves.txt cannot be read.
     */
    explicit Server(std::uint16_t port, Difficulty difficulty = Difficulty::Normal);

    /**
     * @brief Take the commands of the server back from the console.
     */
    ~Server(void) override;

    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) = delete;
    Server &operator=(Server &&) = delete;

protected:
    /**
     * @brief Read the datagrams received since the last frame, run the lines typed in the terminal with the console and
     *        log their answer, and stop the loop when the process was asked to stop.
     */
    void onInput(void) override;

    /**
     * @brief Run one step: apply the inputs, run the systems, play the waves, follow the players, send the snapshot,
     *        the events and the scoreboard. The game stands still while no client is in it, so it starts with the first
     *        Join. Once the game is over, every player out of lives or the boss beaten, the next game starts after
     *        GAME_OVER_DELAY. Every TRAFFIC_LOG_PERIOD, log the bytes sent and received per second.
     *
     * @param step The duration of the step, in seconds
     */
    void onFixedUpdate(float step) override;

    /**
     * @brief Say that the server stopped.
     */
    void onStop(void) override;

private:
    [[nodiscard]] std::vector<PlayerInfo> getPlayers(void) const override;
    bool kickPlayer(std::uint8_t slot) override;
    void restartGame(void) override;
    [[nodiscard]] WaveProgress getWaves(void) const override;
    void stopServer(void) override;
    [[nodiscard]] Difficulty getDifficulty(void) const override;
    void setDifficulty(Difficulty difficulty) override;

    /**
     * @brief Run the lines typed in the terminal with the console, and log their answer.
     */
    void runCommands(void);

    /**
     * @brief A client that joined, and its ship until it is out of lives.
     */
    struct Client
    {
        younity::network::Endpoint endpoint;
        std::optional<younity::ecs::Entity> ship;
        // Keeps the inputs in the order of their step, dropping a duplicated or reordered one
        InputGate inputs;
        // The tick of the newest input accepted, given back in the snapshots
        std::uint32_t lastInput = 0;
    };

    /**
     * @brief Handle a bounded number of the datagrams received since the last step, log the invalid ones with a rate
     *        limit, then let go of the clients silent for the session timeout. Only a client with an open session can
     *        join and play.
     */
    void receive(void);

    /**
     * @brief Tell the drop log about an invalid datagram, and print what it returns.
     *
     * @param reason Why the datagram was rejected
     * @param now The current time
     */
    void logReject(std::string_view reason, std::chrono::steady_clock::time_point now);

    /**
     * @brief Give a slot and a ship to a client asking to join. A client already in the game gets its slot again, a
     *        client finding the four slots taken gets Full.
     *
     * @param from The client
     */
    void onJoin(const younity::network::Endpoint &from);

    /**
     * @brief Free the slot of a client that quit or timed out: remove its ship and show it as left to the others.
     *        When nobody is left, the game starts over and waits for the next players.
     *
     * @param from The client
     */
    void onLeave(const younity::network::Endpoint &from);

    /**
     * @brief Store the inputs of a client in its ship, applied by the PlayerMovementSystem and the ShootSystem. An
     *        input that is not newer than the last one of the client is dropped.
     *
     * @param from The client
     * @param message Its inputs
     */
    void applyInput(const younity::network::Endpoint &from, const InputMessage &message);

    /**
     * @brief Apply what two touching entities do to each other, see resolveCollision().
     *
     * @param event The two entities
     */
    void onCollision(const younity::physics::CollisionEvent &event);

    /**
     * @brief Read the score and the lives of each ship, and forget the ships out of lives.
     *
     * @return true when the game is over: every client still in the game is out of lives.
     */
    bool updatePlayers(void);

    /**
     * @brief Count the ships in the game, to make the waves harder with more players.
     *
     * @return The clients that still have a ship.
     */
    [[nodiscard]] std::size_t countShips(void) const;

    /**
     * @brief Tell whether a client is in the game, with a ship or out of lives.
     *
     * @return false when every slot is free.
     */
    [[nodiscard]] bool hasClients(void) const;

    /**
     * @brief Start a new game: remove every entity, give each client a new ship, forget the players who left
     *        and start the waves again.
     */
    void restart(void);

    /**
     * @brief Send the score and the lives of each slot, and the game over or the victory, to every client.
     *
     * @param tick The step of the server they were read at
     */
    void broadcastScoreboard(std::uint32_t tick);

    /**
     * @brief Send every entity of the game to every client, in as many chunks as a datagram needs, with the last
     *        input of that client. Queue the events of the entities that appeared or went.
     *
     * @param tick The step of the server they were read at
     */
    void broadcastSnapshot(std::uint32_t tick);

    /**
     * @brief Send the events queued during the step to every client, then forget them.
     */
    void broadcastEvents(void);

    /**
     * @brief Queue an event for the current step, unless the step already queues too many.
     *
     * @param event The event to send after the snapshot
     */
    void queueEvent(Message event);

    /**
     * @brief Find the slot of a client.
     *
     * @param endpoint The client
     *
     * @return Its slot, or nothing if it has not joined.
     */
    [[nodiscard]] std::optional<std::uint8_t> findSlot(const younity::network::Endpoint &endpoint) const;

    /**
     * @brief Send a message to a client, numbered by its session. A client without a session gets nothing.
     *
     * @param to The client
     * @param message The message
     */
    void send(const younity::network::Endpoint &to, const Message &message);

    /**
     * @brief Send a datagram and count it in the traffic.
     *
     * @param to Who receives it
     * @param bytes The datagram
     */
    void sendBytes(const younity::network::Endpoint &to, std::span<const std::byte> bytes);

    /**
     * @brief Send a message to every client that joined.
     *
     * @param message The message
     */
    void broadcast(const Message &message);

    younity::network::UdpPump _pump;
    // Before the registry: its systems read the rules until they are destroyed
    Difficulty _difficulty;
    DifficultyRules _rules;
    younity::ecs::Registry _registry;
    younity::EventBus _bus;
    younity::network::SessionTable _sessions;
    std::array<std::optional<Client>, MAX_PLAYERS> _clients;
    // How each slot is doing, kept after its client left so the others see who it was
    ScoreboardMessage _scoreboard;
    // The events of the step, sent after the snapshot
    std::vector<Message> _events;
    // Counts the invalid datagrams and prints a report at most once a second
    DropLog _drops;
    // Finds the events that the state of the game implies
    EventTracker _tracker;
    // The bytes sent and received, logged every TRAFFIC_LOG_PERIOD
    TrafficMeter _traffic{TRAFFIC_LOG_PERIOD};
    // The waves of assets/data/waves.txt, created again for each game
    std::vector<Wave> _waveFile;
    WaveManager _waves;
    // Counts down from GAME_OVER_DELAY once the game is over, empty while it runs
    std::optional<float> _gameOver;
    // The steps run since the server started, one snapshot each
    std::uint32_t _tick = 0;
    // The lines typed in the terminal of the server
    ConsoleInput _input;
    // The clients kicked out: their datagrams are ignored until their session times out
    std::vector<younity::network::Endpoint> _kicked;
};

} // namespace jtype

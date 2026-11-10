/*
** EPITECH PROJECT, 2026
** j-type/client/src/network/Connection
** File description:
** Connection header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "jtype/common/Messages.hpp"
#include "jtype/common/TrafficMeter.hpp"
#include "network/SnapshotAssembler.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Session.hpp"
#include "younity/network/UdpPump.hpp"

namespace jtype {

/**
 * @brief What the server sent since the last call to Connection::receive().
 */
struct Received
{
    // The whole snapshots, in the order they came, which is not always the order of the server
    std::vector<SnapshotMessage> snapshots;
    // The events, in the order they came, to play when the world drawn reaches their step
    std::vector<Message> events;
};

/**
 * @brief The link to the server: it opens a session, asks for a slot, sends the inputs and receives the snapshots
 *        and the scoreboard.
 */
class Connection
{
public:
    /**
     * @brief Open the socket.
     *
     * @param server The server to join
     * @param config The timing of the session, whose timeout is how long the server may stay silent
     *
     * @throw std::runtime_error if the socket cannot be opened.
     */
    explicit Connection(younity::network::Endpoint server, younity::network::SessionConfig config = {});

    /**
     * @brief Handle all the datagrams received since the last call, keep the last scoreboard and put the chunks of the
     *        snapshots back together. Each second, the traffic of the client is published as the metrics
     *        network.bytes_sent, network.bytes_received and network.snapshots, see TrafficMeter.
     *
     * @return The whole snapshots and the events received.
     *
     * @throw std::runtime_error if the server is full, or stayed silent past the session timeout: unreachable before
     *        the slot, lost after it.
     */
    [[nodiscard]] Received receive(void);

    /**
     * @brief Open the session, then ask for a slot, again every JOIN_DELAY while the server has not answered.
     *
     * @param step The duration of the step, in seconds
     */
    void join(float step);

    /**
     * @brief Send the actions held down by the player, with their number. Sent at each step, they also keep the session
     *        open.
     *
     * @param inputs The input bitmask
     *
     * @return Their number, from 1, which the snapshots give back once the ship moved with them.
     */
    [[nodiscard]] std::uint32_t sendInputs(std::uint8_t inputs);

    /**
     * @brief Tell the server the player quits, so the others see it at once instead of after the session timeout.
     *        Does nothing while the client has no slot.
     */
    void leave(void);

    /**
     * @brief Get the slot given by the server.
     *
     * @return The slot, or nothing while the server has not answered.
     */
    [[nodiscard]] std::optional<std::uint8_t> getSlot(void) const;

    /**
     * @brief Get the last scoreboard received.
     *
     * @return The scoreboard, every slot free while none came.
     */
    [[nodiscard]] const ScoreboardMessage &getScoreboard(void) const;

    /**
     * @brief Get the server joined.
     *
     * @return The address of the server.
     */
    [[nodiscard]] const younity::network::Endpoint &getServer(void) const;

private:
    /**
     * @brief Send a datagram and count it in the traffic.
     *
     * @param to Who receives it
     * @param bytes The datagram
     */
    void send(const younity::network::Endpoint &to, std::span<const std::byte> bytes);

    younity::network::Endpoint _server;
    younity::network::UdpPump _pump;
    younity::network::Session _session;
    SnapshotAssembler _assembler;
    std::optional<std::uint8_t> _slot;
    ScoreboardMessage _scoreboard;
    float _joinTimer = 0.0F;
    // The step of the client, sent with each input so the server drops a stale or a duplicated one
    std::uint32_t _tick = 0;
    // Published each second, for the metrics window of the debug layout
    TrafficMeter _traffic;
};

} // namespace jtype

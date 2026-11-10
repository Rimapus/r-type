/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/SessionTable
** File description:
** SessionTable header file
*/

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "younity/network/Datagram.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Packet.hpp"

namespace younity::network {

/**
 * @brief The peers a server currently plays with, keyed by endpoint.
 *
 * Pure logic without any thread or socket: the game owns an UdpPump, polls
 * it once per fixed step, decodes what arrives and feeds the packets here,
 * and sends back the returned answers. Times are given by the caller as
 * steady clock points, so the table never reads the clock itself and the
 * tests use exact times without waiting.
 */
class SessionTable
{
public:
    /**
     * @brief Create an empty table with the given session timeout and capacity.
     *
     * @param timeout The delay without any packet from a peer before sweep drops it
     * @param maxSessions The largest number of peers kept at once
     */
    explicit SessionTable(
        std::chrono::milliseconds timeout = DEFAULT_SESSION_TIMEOUT,
        std::size_t maxSessions = MAX_SESSIONS) noexcept;

    /**
     * @brief Feed a decoded packet from a peer.
     *
     * A Connect registers a new peer or refreshes a known one, and always
     * answers a ConnectAck: answering twice is harmless and covers a lost
     * answer. A Heartbeat from a known peer refreshes it and is answered by
     * a Heartbeat, so the peer keeps hearing from the server while the game
     * has nothing to send. A Ping is answered by a Pong with the same cookie,
     * and any other packet from a known peer only refreshes it. Anything from
     * an unknown peer is dropped without an answer.
     *
     * Every answer carries the next sequence number of the peer, increasing
     * modulo 2^32, so the client can count the packets it misses; the packets
     * of the game take theirs from the same counter with nextSequence(). A Connect
     * restarts the counter at 0, and a Connect from a new peer is dropped
     * without an answer once the table is full.
     *
     * @param from Who sent the packet
     * @param packet The decoded packet
     * @param now The current time, stamps the last activity
     *
     * @return The ConnectAck, Heartbeat or Pong send request, or nothing when
     *         no answer is due.
     */
    [[nodiscard]] std::optional<SendRequest> onPacket(
        const Endpoint &from, const Packet &packet, std::chrono::steady_clock::time_point now);

    /**
     * @brief Drop the peers silent past the timeout.
     *
     * @param now The current time
     *
     * @return The dropped peers, so the game can react to their departure.
     */
    [[nodiscard]] std::vector<Endpoint> sweep(std::chrono::steady_clock::time_point now);

    /**
     * @brief Take the next sequence number of a peer, for a packet the game sends it, e.g. a GameData.
     *        The session answers take theirs from the same counter, so the peer sees one sequence without holes
     *        and counts a hole as a loss only when a packet was lost.
     *
     * @param peer The peer the packet goes to
     *
     * @return The sequence number, or nothing if the peer has no session.
     */
    [[nodiscard]] std::optional<std::uint32_t> nextSequence(const Endpoint &peer);

    /**
     * @brief Check if a peer currently has a session.
     *
     * @param peer The peer to look for
     *
     * @return true from its Connect until sweep drops it.
     */
    [[nodiscard]] bool isConnected(const Endpoint &peer) const;

    /**
     * @brief Get the number of current sessions.
     *
     * @return The number of peers in the table.
     */
    [[nodiscard]] std::size_t getSize(void) const noexcept;

private:
    /**
     * @brief One peer of the table with its last activity and the next sequence number to send.
     */
    struct Entry
    {
        Endpoint peer = Endpoint("0.0.0.0", 0);
        std::chrono::steady_clock::time_point lastSeen{};
        std::uint32_t nextSeq = 0;
    };

    std::chrono::milliseconds _timeout = DEFAULT_SESSION_TIMEOUT;
    std::size_t _maxSessions = MAX_SESSIONS;
    std::map<std::string, Entry> _sessions;
};

} // namespace younity::network

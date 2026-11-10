/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/Session
** File description:
** Session header file
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <span>
#include <string_view>
#include <utility>

#include "younity/network/Datagram.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Packet.hpp"

namespace younity::tools {
class MetricsStore;
}

namespace younity::network {

/**
 * @brief The state of a client session with one server.
 *
 * Disconnected is the initial state and the state after disconnect. Connecting
 * repeats the Connect until the server answers. Connected exchanges game
 * data and heartbeats. TimedOut is entered when the server stays silent past
 * the timeout and sticks until the next connect or disconnect.
 */
enum class SessionState {
    Disconnected,
    Connecting,
    Connected,
    TimedOut
};

/**
 * @brief Get a short human-readable description of a session state.
 *
 * @param state The state to describe
 *
 * @return The description, e.g. "connected".
 */
[[nodiscard]] std::string_view toString(SessionState state) noexcept;

/**
 * @brief The timing of a session: how often to send a heartbeat or a ping and
 *        how long to wait for the peer before declaring it gone.
 *
 * The timeout should hold several heartbeat intervals, so a single lost
 * datagram never drops a healthy session. The heartbeat interval also paces
 * the Connect repeated while connecting. The ping interval paces the round
 * trips used for the latency metric.
 */
struct SessionConfig
{
    std::chrono::milliseconds heartbeatInterval = DEFAULT_HEARTBEAT_INTERVAL;
    std::chrono::milliseconds timeout = DEFAULT_SESSION_TIMEOUT;
    std::chrono::milliseconds pingInterval = DEFAULT_PING_INTERVAL;
};

/**
 * @brief One client session with one server, driven by the game loop.
 *
 * Pure logic without any thread or socket: the game owns an UdpPump, polls
 * it once per fixed step, decodes what arrives and feeds the packets here,
 * and sends back whatever connect and pollHeartbeat return. Times are given
 * by the caller as steady clock points, so the session never reads the clock
 * itself and the tests use exact times without waiting.
 *
 * The session publishes two metrics in a tools::MetricsStore: "network.latency",
 * a Timer holding the round trip of each answered ping, and "network.packets_lost",
 * a Counter holding the missing sequence numbers of the server. Recording is best
 * effort: a store that fails never interrupts the session.
 */
class Session
{
public:
    /**
     * @brief Create a disconnected session with the default timing and the shared metrics store.
     */
    Session(void);

    /**
     * @brief Create a disconnected session with a custom timing and the shared metrics store.
     *
     * @param config The heartbeat interval, the timeout and the ping interval
     */
    explicit Session(SessionConfig config);

    /**
     * @brief Create a disconnected session with the default timing and another metrics store.
     *
     * @param metrics The store receiving the network metrics, which must outlive the session
     */
    explicit Session(tools::MetricsStore &metrics) noexcept;

    /**
     * @brief Create a disconnected session with a custom timing and another metrics store.
     *
     * @param config The heartbeat interval, the timeout and the ping interval
     * @param metrics The store receiving the network metrics, which must outlive the session
     */
    Session(SessionConfig config, tools::MetricsStore &metrics) noexcept;

    /**
     * @brief Get the current state.
     *
     * @return The state, Disconnected before the first connect.
     */
    [[nodiscard]] SessionState getState(void) const noexcept;

    /**
     * @brief Open a session with a server.
     *
     * Moves to Connecting even from Connected or TimedOut: calling it again
     * restarts the handshake from scratch.
     *
     * @param server The server to talk to
     * @param now The current time, stamps the last activity
     *
     * @return The Connect datagram to send to the server.
     */
    [[nodiscard]] std::optional<SendRequest> connect(
        const Endpoint &server, std::chrono::steady_clock::time_point now);

    /**
     * @brief Feed a decoded packet from the server.
     *
     * Packets from anyone else are ignored. While Connecting, a ConnectAck
     * or a Heartbeat moves to Connected; while Connected, any packet proves
     * the server is alive. Anything arriving while Disconnected or TimedOut
     * is ignored. An accepted packet advances the loss tracking, and a Pong
     * whose cookie matches an outstanding ping publishes its round trip.
     *
     * @param from Who sent the packet
     * @param packet The decoded packet
     * @param now The current time, stamps the last activity
     */
    void onPacket(const Endpoint &from, const Packet &packet, std::chrono::steady_clock::time_point now) noexcept;

    /**
     * @brief Get the packet that keeps the session going when one is due.
     *
     * While Connecting it is the Connect again, so a lost Connect or
     * ConnectAck only delays the handshake. While Connected it is a
     * Heartbeat when one is due, otherwise a Ping when its own interval
     * elapsed: a heartbeat comes first so the keepalive is never delayed.
     *
     * @param now The current time
     *
     * @return The Connect, Heartbeat or Ping datagram for the server, or
     *         nothing while disconnected, timed out or before an interval
     *         elapsed.
     */
    [[nodiscard]] std::optional<SendRequest> pollHeartbeat(std::chrono::steady_clock::time_point now);

    /**
     * @brief Move to TimedOut when the server stayed silent past the timeout.
     *
     * Only leaves Connecting and Connected: TimedOut sticks until the next
     * connect or disconnect, and Disconnected never times out.
     *
     * @param now The current time
     *
     * @return The state after the check.
     */
    [[nodiscard]] SessionState update(std::chrono::steady_clock::time_point now) noexcept;

    /**
     * @brief Take the next sequence number of the client, for a packet the game sends to the server, e.g. a GameData.
     *        The session packets take theirs from the same counter, so the server sees one sequence without holes.
     *
     * @return The sequence number.
     */
    [[nodiscard]] std::uint32_t nextSequence(void) noexcept;

    /**
     * @brief Forget the session and move back to Disconnected.
     */
    void disconnect(void) noexcept;

private:
    /**
     * @brief Encode one packet for the server with the next sequence number.
     *
     * @param type The packet type
     * @param payload The payload, empty for a session packet
     *
     * @return The send request, or nothing if the encoding failed.
     */
    [[nodiscard]] std::optional<SendRequest> makeSessionPacket(
        PacketType type, std::span<const std::byte> payload = {});

    /**
     * @brief Build a Ping with a new cookie and remember when it was sent.
     *
     * @param now The current time, the start of the round trip
     *
     * @return The send request, or nothing if the encoding failed.
     */
    [[nodiscard]] std::optional<SendRequest> makePingPacket(std::chrono::steady_clock::time_point now);

    /**
     * @brief Record the round trip of a Pong, when its cookie matches an outstanding ping.
     *
     * @param packet The Pong received
     * @param now The current time, the end of the round trip
     */
    void onPong(const Packet &packet, std::chrono::steady_clock::time_point now) noexcept;

    /**
     * @brief Advance the loss tracking with the sequence number of a packet from the server.
     *
     * @param seq The sequence number of the received packet
     */
    void trackServerSequence(std::uint32_t seq) noexcept;

    SessionConfig _config;
    SessionState _state = SessionState::Disconnected;
    Endpoint _server = Endpoint("0.0.0.0", 0);
    tools::MetricsStore &_metrics;
    std::uint32_t _nextSeq = 0;
    std::uint32_t _nextPingCookie = 0;
    std::uint32_t _lastServerSeq = 0;
    bool _hasServerSeq = false;
    std::deque<std::pair<std::uint32_t, std::chrono::steady_clock::time_point>> _outstandingPings;
    std::chrono::steady_clock::time_point _lastSeen{};
    std::chrono::steady_clock::time_point _lastHeartbeat{};
    std::chrono::steady_clock::time_point _lastPing{};
};

} // namespace younity::network

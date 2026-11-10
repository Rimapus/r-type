/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/Session
** File description:
** Session source file
*/

#include "younity/network/Session.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "SessionPacket.hpp"
#include "younity/network/Reader.hpp"
#include "younity/network/Writer.hpp"
#include "younity/tools/Global.hpp"
#include "younity/tools/Metric.hpp"
#include "younity/tools/MetricsStore.hpp"

namespace younity::network {

namespace {

// Name of the timer holding the round trip of each answered ping
constexpr const char *LATENCY_METRIC = "network.latency";
// Name of the counter holding the missing sequence numbers of the server
constexpr const char *PACKETS_LOST_METRIC = "network.packets_lost";
// Pings kept while their answer is missing: stops an endless loss from growing the list forever
constexpr std::size_t MAX_OUTSTANDING_PINGS = 8;
// A sequence number is newer when it is less than half the u32 range ahead of the last one
constexpr std::uint32_t SEQUENCE_HALF_RANGE = 0x80000000U;

} // namespace

std::string_view toString(SessionState state) noexcept
{
    switch (state) {
    case SessionState::Disconnected:
        return "disconnected";
    case SessionState::Connecting:
        return "connecting";
    case SessionState::Connected:
        return "connected";
    case SessionState::TimedOut:
        return "timed out";
    }
    return "unknown";
}

Session::Session(void)
    : _metrics(tools::metrics())
{
}

Session::Session(SessionConfig config)
    : _config(config), _metrics(tools::metrics())
{
}

Session::Session(tools::MetricsStore &metrics) noexcept
    : _metrics(metrics)
{
}

Session::Session(SessionConfig config, tools::MetricsStore &metrics) noexcept
    : _config(config), _metrics(metrics)
{
}

SessionState Session::getState(void) const noexcept
{
    return this->_state;
}

std::optional<SendRequest> Session::connect(const Endpoint &server, std::chrono::steady_clock::time_point now)
{
    this->_server = server;
    this->_state = SessionState::Connecting;
    this->_lastSeen = now;
    this->_lastHeartbeat = now;
    this->_lastPing = now;
    this->_hasServerSeq = false;
    this->_outstandingPings.clear();
    return this->makeSessionPacket(PacketType::Connect);
}

void Session::onPacket(
    const Endpoint &from, const Packet &packet, std::chrono::steady_clock::time_point now) noexcept
{
    if (from != this->_server) {
        return;
    }
    if (this->_state == SessionState::Connecting &&
        (packet.type == PacketType::ConnectAck || packet.type == PacketType::Heartbeat)) {
        this->_state = SessionState::Connected;
    } else if (this->_state != SessionState::Connected) {
        return;
    }
    this->_lastSeen = now;
    this->trackServerSequence(packet.seq);
    if (packet.type == PacketType::Pong) {
        this->onPong(packet, now);
    }
}

std::optional<SendRequest> Session::pollHeartbeat(std::chrono::steady_clock::time_point now)
{
    if (this->_state != SessionState::Connecting && this->_state != SessionState::Connected) {
        return std::nullopt;
    }
    if (now - this->_lastHeartbeat >= this->_config.heartbeatInterval) {
        this->_lastHeartbeat = now;
        // A lost Connect or ConnectAck must not fail the handshake: ask again
        // until the server answers.
        if (this->_state == SessionState::Connecting) {
            return this->makeSessionPacket(PacketType::Connect);
        }
        return this->makeSessionPacket(PacketType::Heartbeat);
    }
    // The keepalive comes first: a ping waits for the next call when both are due.
    if (this->_state == SessionState::Connected && now - this->_lastPing >= this->_config.pingInterval) {
        this->_lastPing = now;
        return this->makePingPacket(now);
    }
    return std::nullopt;
}

SessionState Session::update(std::chrono::steady_clock::time_point now) noexcept
{
    if ((this->_state == SessionState::Connecting || this->_state == SessionState::Connected) &&
        now - this->_lastSeen >= this->_config.timeout) {
        this->_state = SessionState::TimedOut;
    }
    return this->_state;
}

std::uint32_t Session::nextSequence(void) noexcept
{
    return this->_nextSeq++;
}

void Session::disconnect(void) noexcept
{
    this->_state = SessionState::Disconnected;
    this->_hasServerSeq = false;
    this->_outstandingPings.clear();
}

std::optional<SendRequest> Session::makeSessionPacket(PacketType type, std::span<const std::byte> payload)
{
    const std::optional<SendRequest> request = encodeSessionPacket(type, this->_server, this->_nextSeq, payload);

    if (request.has_value()) {
        this->_nextSeq++;
    }
    return request;
}

std::optional<SendRequest> Session::makePingPacket(std::chrono::steady_clock::time_point now)
{
    const std::uint32_t cookie = this->_nextPingCookie++;
    Writer payload;

    if (!payload.putU32Le(cookie)) {
        return std::nullopt;
    }
    const std::optional<SendRequest> request = this->makeSessionPacket(PacketType::Ping, payload.getData());

    if (!request.has_value()) {
        return std::nullopt;
    }
    if (this->_outstandingPings.size() >= MAX_OUTSTANDING_PINGS) {
        this->_outstandingPings.pop_front();
    }
    this->_outstandingPings.emplace_back(cookie, now);
    return request;
}

void Session::onPong(const Packet &packet, std::chrono::steady_clock::time_point now) noexcept
{
    Reader in(packet.payload);
    const std::expected<std::uint32_t, CodecError> cookie = in.getU32Le();

    if (!cookie.has_value()) {
        return;
    }
    const auto found = std::find_if(
        this->_outstandingPings.begin(), this->_outstandingPings.end(),
        [cookie = *cookie](const auto &entry) { return entry.first == cookie; });

    if (found == this->_outstandingPings.end()) {
        return;
    }
    const double roundTrip = std::chrono::duration<double>(now - found->second).count();

    this->_outstandingPings.erase(found);
    if (roundTrip >= 0.0) {
        // Best effort: a metric that cannot be recorded must not break the session
        try {
            this->_metrics.record(LATENCY_METRIC, roundTrip, tools::MetricKind::Timer);
        } catch (...) {
        }
    }
}

void Session::trackServerSequence(std::uint32_t seq) noexcept
{
    if (!this->_hasServerSeq) {
        this->_lastServerSeq = seq;
        this->_hasServerSeq = true;
        return;
    }
    const std::uint32_t distance = seq - this->_lastServerSeq;

    // Zero is a duplicate, and more than half the u32 range means an older
    // packet: neither tells anything about a loss.
    if (distance == 0 || distance > SEQUENCE_HALF_RANGE) {
        return;
    }
    const std::uint32_t lost = distance - 1;

    if (lost > 0) {
        // Best effort: a metric that cannot be recorded must not break the session
        try {
            this->_metrics.increment(PACKETS_LOST_METRIC, static_cast<double>(lost));
        } catch (...) {
        }
    }
    this->_lastServerSeq = seq;
}

} // namespace younity::network

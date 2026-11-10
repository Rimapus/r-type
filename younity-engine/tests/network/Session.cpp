/*
** EPITECH PROJECT, 2026
** younity-engine/tests/network/Session
** File description:
** Session unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <string_view>
#include <vector>

#include "younity/network/Datagram.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Packet.hpp"
#include "younity/network/PacketError.hpp"
#include "younity/network/PacketType.hpp"
#include "younity/network/Session.hpp"
#include "younity/tools/Metric.hpp"
#include "younity/tools/MetricsStore.hpp"

using younity::network::Endpoint;
using younity::network::Packet;
using younity::network::PacketError;
using younity::network::PacketType;
using younity::network::SendRequest;
using younity::network::Session;
using younity::network::SessionConfig;
using younity::network::SessionState;
using younity::tools::MetricKind;
using younity::tools::MetricsStore;

namespace {

const std::chrono::steady_clock::time_point START{};

Packet decodeOrFail(const std::vector<std::byte> &bytes)
{
    const std::expected<Packet, PacketError> packet = younity::network::decodePacket(bytes);

    cr_assert(packet.has_value());
    return *packet;
}

Packet makePacket(PacketType type, std::uint32_t seq = 0)
{
    return Packet{.type = type, .seq = seq, .payload = {}};
}

Packet makePong(const std::vector<std::byte> &cookie, std::uint32_t seq)
{
    return Packet{.type = PacketType::Pong, .seq = seq, .payload = cookie};
}

} // namespace

Test(network_session, describes_all_states)
{
    cr_assert_eq(younity::network::toString(SessionState::Disconnected), std::string_view("disconnected"));
    cr_assert_eq(younity::network::toString(SessionState::Connecting), std::string_view("connecting"));
    cr_assert_eq(younity::network::toString(SessionState::Connected), std::string_view("connected"));
    cr_assert_eq(younity::network::toString(SessionState::TimedOut), std::string_view("timed out"));
}

Test(network_session, starts_disconnected)
{
    Session session;

    cr_assert_eq(session.getState(), SessionState::Disconnected);
    cr_assert_eq(session.update(START), SessionState::Disconnected);
    cr_assert_not(session.pollHeartbeat(START).has_value());
}

Test(network_session, connect_moves_to_connecting_with_connect_bytes)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);
    const std::optional<SendRequest> request = session.connect(server, START);

    cr_assert_eq(session.getState(), SessionState::Connecting);
    cr_assert(request.has_value());
    cr_assert(request->to == server);

    const Packet packet = decodeOrFail(request->bytes);

    cr_assert_eq(packet.type, PacketType::Connect);
    cr_assert(packet.payload.empty());
}

Test(network_session, connect_ack_from_server_connects)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START + std::chrono::milliseconds{10});
    cr_assert_eq(session.getState(), SessionState::Connected);
}

Test(network_session, ack_from_a_stranger_is_ignored)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(Endpoint::loopback(9999), makePacket(PacketType::ConnectAck), START + std::chrono::milliseconds{10});
    cr_assert_eq(session.getState(), SessionState::Connecting);
}

Test(network_session, heartbeat_from_server_while_connecting_connects)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::Heartbeat), START + std::chrono::milliseconds{10});
    cr_assert_eq(session.getState(), SessionState::Connected);
}

Test(network_session, heartbeat_is_sent_only_when_due)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    cr_assert_not(session.pollHeartbeat(START + std::chrono::milliseconds{499}).has_value());

    const std::optional<SendRequest> request = session.pollHeartbeat(START + std::chrono::milliseconds{500});

    cr_assert(request.has_value());
    cr_assert(request->to == server);
    cr_assert_eq(decodeOrFail(request->bytes).type, PacketType::Heartbeat);
    cr_assert_not(session.pollHeartbeat(START + std::chrono::milliseconds{500}).has_value());
}

Test(network_session, connect_is_sent_again_while_connecting)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    cr_assert_not(session.pollHeartbeat(START + std::chrono::milliseconds{499}).has_value());

    const std::optional<SendRequest> request = session.pollHeartbeat(START + std::chrono::milliseconds{500});

    cr_assert(request.has_value());
    cr_assert(request->to == server);
    cr_assert_eq(decodeOrFail(request->bytes).type, PacketType::Connect);
    cr_assert_eq(session.getState(), SessionState::Connecting);
}

Test(network_session, nothing_is_sent_after_a_timeout)
{
    Session session;

    (void)session.connect(Endpoint::loopback(4242), START);
    cr_assert_eq(session.update(START + std::chrono::milliseconds{3000}), SessionState::TimedOut);
    cr_assert_not(session.pollHeartbeat(START + std::chrono::milliseconds{5000}).has_value());
}

Test(network_session, sequence_numbers_increase)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);
    const std::optional<SendRequest> connect = session.connect(server, START);

    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    const std::optional<SendRequest> heartbeat = session.pollHeartbeat(START + std::chrono::milliseconds{500});

    cr_assert(connect.has_value());
    cr_assert(heartbeat.has_value());
    cr_assert_neq(decodeOrFail(connect->bytes).seq, decodeOrFail(heartbeat->bytes).seq);
}

Test(network_session, game_packets_share_the_sequence_of_the_session)
{
    Session session;
    const std::optional<SendRequest> connect = session.connect(Endpoint::loopback(4242), START);
    const std::uint32_t game = session.nextSequence();
    const std::optional<SendRequest> again = session.pollHeartbeat(START + std::chrono::milliseconds{500});

    cr_assert(again.has_value());
    cr_assert_eq(game, decodeOrFail(connect->bytes).seq + 1);
    cr_assert_eq(decodeOrFail(again->bytes).seq, game + 1);
}

Test(network_session, gamedata_refreshes_liveness)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    session.onPacket(server, makePacket(PacketType::GameData), START + std::chrono::milliseconds{2500});
    cr_assert_eq(session.update(START + std::chrono::milliseconds{5499}), SessionState::Connected);
    cr_assert_eq(session.update(START + std::chrono::milliseconds{5500}), SessionState::TimedOut);
}

Test(network_session, connecting_times_out_without_answer)
{
    Session session;

    (void)session.connect(Endpoint::loopback(4242), START);
    cr_assert_eq(session.update(START + std::chrono::milliseconds{2999}), SessionState::Connecting);
    cr_assert_eq(session.update(START + std::chrono::milliseconds{3000}), SessionState::TimedOut);
}

Test(network_session, timeout_sticks_until_reconnect)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    cr_assert_eq(session.update(START + std::chrono::milliseconds{3000}), SessionState::TimedOut);
    session.onPacket(server, makePacket(PacketType::Heartbeat), START + std::chrono::milliseconds{3001});
    cr_assert_eq(session.getState(), SessionState::TimedOut);
    cr_assert_eq(session.update(START + std::chrono::milliseconds{6000}), SessionState::TimedOut);
}

Test(network_session, disconnect_resets)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    session.disconnect();
    cr_assert_eq(session.getState(), SessionState::Disconnected);
    cr_assert_not(session.pollHeartbeat(START + std::chrono::milliseconds{5000}).has_value());
    cr_assert_eq(session.update(START + std::chrono::milliseconds{5000}), SessionState::Disconnected);
}

Test(network_session, reconnect_restarts_the_handshake)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    const std::optional<SendRequest> request = session.connect(server, START + std::chrono::milliseconds{10});

    cr_assert_eq(session.getState(), SessionState::Connecting);
    cr_assert(request.has_value());
    cr_assert_eq(decodeOrFail(request->bytes).type, PacketType::Connect);
}

Test(network_session, custom_timing_is_used)
{
    Session session(SessionConfig{
        .heartbeatInterval = std::chrono::milliseconds{50}, .timeout = std::chrono::milliseconds{200}});
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    cr_assert(session.pollHeartbeat(START + std::chrono::milliseconds{50}).has_value());
    cr_assert_eq(session.update(START + std::chrono::milliseconds{200}), SessionState::TimedOut);
}

Test(network_session, sends_a_ping_when_it_is_due)
{
    MetricsStore metrics;
    Session session(
        SessionConfig{
            .heartbeatInterval = std::chrono::milliseconds{5000},
            .timeout = std::chrono::milliseconds{10000},
            .pingInterval = std::chrono::milliseconds{1000}},
        metrics);
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    cr_assert_not(session.pollHeartbeat(START + std::chrono::milliseconds{999}).has_value());

    const std::optional<SendRequest> request = session.pollHeartbeat(START + std::chrono::milliseconds{1000});

    cr_assert(request.has_value());
    cr_assert(request->to == server);
    const Packet ping = decodeOrFail(request->bytes);

    cr_assert_eq(ping.type, PacketType::Ping);
    cr_assert_eq(ping.payload.size(), younity::network::PING_COOKIE_BYTES);
    cr_assert_not(session.pollHeartbeat(START + std::chrono::milliseconds{1000}).has_value());
}

Test(network_session, a_heartbeat_comes_before_a_ping_when_both_are_due)
{
    Session session;
    const Endpoint server = Endpoint::loopback(4242);
    const std::chrono::steady_clock::time_point later = START + std::chrono::milliseconds{1000};

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);

    const std::optional<SendRequest> heartbeat = session.pollHeartbeat(later);
    const std::optional<SendRequest> ping = session.pollHeartbeat(later);

    cr_assert(heartbeat.has_value());
    cr_assert(ping.has_value());
    cr_assert_eq(decodeOrFail(heartbeat->bytes).type, PacketType::Heartbeat);
    cr_assert_eq(decodeOrFail(ping->bytes).type, PacketType::Ping);
}

Test(network_session, publishes_the_latency_of_an_answered_ping)
{
    MetricsStore metrics;
    Session session(
        SessionConfig{
            .heartbeatInterval = std::chrono::milliseconds{5000},
            .timeout = std::chrono::milliseconds{10000},
            .pingInterval = std::chrono::milliseconds{1000}},
        metrics);
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);

    const std::optional<SendRequest> request = session.pollHeartbeat(START + std::chrono::milliseconds{1000});

    cr_assert(request.has_value());
    const Packet ping = decodeOrFail(request->bytes);

    session.onPacket(server, makePong(ping.payload, 1), START + std::chrono::milliseconds{1100});
    const std::optional<younity::tools::Metric> latency = metrics.getMetric("network.latency");

    cr_assert(latency.has_value());
    cr_assert_eq(latency->kind, MetricKind::Timer);
    cr_assert_float_eq(latency->last, 0.1, 0.000001);
}

Test(network_session, ignores_a_pong_without_an_outstanding_ping)
{
    MetricsStore metrics;
    Session session(metrics);
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    session.onPacket(
        server, makePong(std::vector<std::byte>(younity::network::PING_COOKIE_BYTES), 1),
        START + std::chrono::milliseconds{10});
    cr_assert_not(metrics.getMetric("network.latency").has_value());
}

Test(network_session, counts_the_missing_sequence_numbers)
{
    MetricsStore metrics;
    Session session(metrics);
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    session.onPacket(server, makePacket(PacketType::Heartbeat, 3), START + std::chrono::milliseconds{10});
    const std::optional<younity::tools::Metric> lost = metrics.getMetric("network.packets_lost");

    cr_assert(lost.has_value());
    cr_assert_eq(lost->kind, MetricKind::Counter);
    cr_assert_float_eq(lost->last, 2.0, 0.000001);

    session.onPacket(server, makePacket(PacketType::Heartbeat, 4), START + std::chrono::milliseconds{20});
    cr_assert_float_eq(metrics.getMetric("network.packets_lost")->last, 2.0, 0.000001);
}

Test(network_session, ignores_duplicate_and_older_sequence_numbers)
{
    MetricsStore metrics;
    Session session(metrics);
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    session.onPacket(server, makePacket(PacketType::Heartbeat, 2), START + std::chrono::milliseconds{10});
    cr_assert_float_eq(metrics.getMetric("network.packets_lost")->last, 1.0, 0.000001);

    session.onPacket(server, makePacket(PacketType::Heartbeat, 1), START + std::chrono::milliseconds{20});
    session.onPacket(server, makePacket(PacketType::Heartbeat, 2), START + std::chrono::milliseconds{30});
    session.onPacket(server, makePacket(PacketType::Heartbeat, 3), START + std::chrono::milliseconds{40});
    cr_assert_float_eq(metrics.getMetric("network.packets_lost")->last, 1.0, 0.000001);
}

Test(network_session, counts_loss_across_the_sequence_wrap)
{
    MetricsStore metrics;
    Session session(metrics);
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(
        server, makePacket(PacketType::ConnectAck, std::numeric_limits<std::uint32_t>::max()), START);
    session.onPacket(server, makePacket(PacketType::Heartbeat, 0), START + std::chrono::milliseconds{10});
    cr_assert_not(metrics.getMetric("network.packets_lost").has_value());

    session.onPacket(server, makePacket(PacketType::Heartbeat, 2), START + std::chrono::milliseconds{20});
    cr_assert_float_eq(metrics.getMetric("network.packets_lost")->last, 1.0, 0.000001);
}

Test(network_session, reconnect_restarts_the_loss_tracking)
{
    MetricsStore metrics;
    Session session(metrics);
    const Endpoint server = Endpoint::loopback(4242);

    (void)session.connect(server, START);
    session.onPacket(server, makePacket(PacketType::ConnectAck), START);
    session.onPacket(server, makePacket(PacketType::Heartbeat, 5), START + std::chrono::milliseconds{10});
    cr_assert_float_eq(metrics.getMetric("network.packets_lost")->last, 4.0, 0.000001);

    session.disconnect();
    (void)session.connect(server, START + std::chrono::milliseconds{1000});
    session.onPacket(server, makePacket(PacketType::ConnectAck), START + std::chrono::milliseconds{1000});
    cr_assert_float_eq(metrics.getMetric("network.packets_lost")->last, 4.0, 0.000001);

    session.onPacket(server, makePacket(PacketType::Heartbeat, 1), START + std::chrono::milliseconds{1010});
    cr_assert_float_eq(metrics.getMetric("network.packets_lost")->last, 4.0, 0.000001);
}

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/network/SessionTable
** File description:
** SessionTable unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <utility>
#include <vector>

#include "younity/network/Datagram.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Packet.hpp"
#include "younity/network/PacketError.hpp"
#include "younity/network/PacketType.hpp"
#include "younity/network/Session.hpp"
#include "younity/network/SessionTable.hpp"
#include "younity/tools/Metric.hpp"
#include "younity/tools/MetricsStore.hpp"

using younity::network::Endpoint;
using younity::network::MAX_SESSIONS;
using younity::network::Packet;
using younity::network::PacketError;
using younity::network::PacketType;
using younity::network::SendRequest;
using younity::network::Session;
using younity::network::SessionConfig;
using younity::network::SessionState;
using younity::network::SessionTable;
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

std::vector<std::byte> makeCookie(void)
{
    return std::vector<std::byte>{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}};
}

Packet makePing(std::vector<std::byte> cookie, std::uint32_t seq = 0)
{
    return Packet{.type = PacketType::Ping, .seq = seq, .payload = std::move(cookie)};
}

} // namespace

Test(network_session_table, starts_empty)
{
    SessionTable table;

    cr_assert_eq(table.getSize(), std::size_t{0});
    cr_assert_not(table.isConnected(Endpoint::loopback(40001)));
    cr_assert(table.sweep(START).empty());
}

Test(network_session_table, connect_registers_and_answers)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);
    const std::optional<SendRequest> answer = table.onPacket(peer, makePacket(PacketType::Connect), START);

    cr_assert(answer.has_value());
    cr_assert(answer->to == peer);
    cr_assert_eq(decodeOrFail(answer->bytes).type, PacketType::ConnectAck);
    cr_assert(table.isConnected(peer));
    cr_assert_eq(table.getSize(), std::size_t{1});
}

Test(network_session_table, reconnect_answers_again_without_duplicating)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);

    cr_assert(table.onPacket(peer, makePacket(PacketType::Connect), START).has_value());
    cr_assert(table.onPacket(peer, makePacket(PacketType::Connect), START + std::chrono::milliseconds{10}).has_value());
    cr_assert_eq(table.getSize(), std::size_t{1});
    cr_assert(table.isConnected(peer));
}

Test(network_session_table, new_peer_is_refused_when_full)
{
    SessionTable table(std::chrono::milliseconds{1000}, 1);
    const Endpoint first = Endpoint::loopback(40001);
    const Endpoint second = Endpoint::loopback(40002);

    cr_assert(table.onPacket(first, makePacket(PacketType::Connect), START).has_value());
    cr_assert_not(table.onPacket(second, makePacket(PacketType::Connect), START).has_value());
    cr_assert_eq(table.getSize(), std::size_t{1});
    cr_assert(table.isConnected(first));
    cr_assert_not(table.isConnected(second));
}

Test(network_session_table, known_peer_reconnects_when_full)
{
    SessionTable table(std::chrono::milliseconds{1000}, 1);
    const Endpoint peer = Endpoint::loopback(40001);

    const std::optional<SendRequest> first = table.onPacket(peer, makePacket(PacketType::Connect), START);
    const std::optional<SendRequest> again =
        table.onPacket(peer, makePacket(PacketType::Connect), START + std::chrono::milliseconds{10});

    cr_assert(first.has_value());
    cr_assert(again.has_value());
    cr_assert_eq(decodeOrFail(again->bytes).type, PacketType::ConnectAck);
    cr_assert_eq(decodeOrFail(again->bytes).seq, 0U);
    cr_assert_eq(table.getSize(), std::size_t{1});
}

Test(network_session_table, sweep_frees_a_slot_for_a_new_peer)
{
    const std::chrono::milliseconds timeout{1000};
    SessionTable table(timeout, 1);
    const Endpoint first = Endpoint::loopback(40001);
    const Endpoint second = Endpoint::loopback(40002);
    const std::chrono::steady_clock::time_point later = START + timeout;

    cr_assert(table.onPacket(first, makePacket(PacketType::Connect), START).has_value());
    cr_assert_not(table.onPacket(second, makePacket(PacketType::Connect), START).has_value());
    cr_assert_eq(table.sweep(later).size(), std::size_t{1});
    cr_assert(table.onPacket(second, makePacket(PacketType::Connect), later).has_value());
    cr_assert(table.isConnected(second));
    cr_assert_eq(table.getSize(), std::size_t{1});
}

Test(network_session_table, default_capacity_refuses_the_extra_peer)
{
    SessionTable table;

    for (std::size_t index = 0; index < MAX_SESSIONS; index += 1) {
        const Endpoint peer = Endpoint::loopback(static_cast<std::uint16_t>(40000 + index));

        cr_assert(table.onPacket(peer, makePacket(PacketType::Connect), START).has_value());
    }
    cr_assert_eq(table.getSize(), MAX_SESSIONS);
    cr_assert_not(table.onPacket(Endpoint::loopback(1), makePacket(PacketType::Connect), START).has_value());
    cr_assert_eq(table.getSize(), MAX_SESSIONS);
}

Test(network_session_table, heartbeat_refreshes_and_is_answered)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);

    (void)table.onPacket(peer, makePacket(PacketType::Connect), START);

    const std::optional<SendRequest> answer =
        table.onPacket(peer, makePacket(PacketType::Heartbeat), START + std::chrono::milliseconds{2500});

    cr_assert(answer.has_value());
    cr_assert(answer->to == peer);
    cr_assert_eq(decodeOrFail(answer->bytes).type, PacketType::Heartbeat);
    cr_assert(table.sweep(START + std::chrono::milliseconds{5499}).empty());
    cr_assert(table.isConnected(peer));
    cr_assert_eq(table.sweep(START + std::chrono::milliseconds{5500}).size(), std::size_t{1});
    cr_assert_not(table.isConnected(peer));
}

Test(network_session_table, gamedata_from_a_known_peer_refreshes_silently)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);

    (void)table.onPacket(peer, makePacket(PacketType::Connect), START);
    cr_assert_not(table.onPacket(peer, makePacket(PacketType::GameData), START + std::chrono::milliseconds{2500}));
    cr_assert(table.isConnected(peer));
    cr_assert_eq(table.getSize(), std::size_t{1});
}

Test(network_session_table, packet_from_a_stranger_is_dropped)
{
    SessionTable table;
    const Endpoint stranger = Endpoint::loopback(40002);

    cr_assert_not(table.onPacket(stranger, makePacket(PacketType::GameData), START).has_value());
    cr_assert_not(table.onPacket(stranger, makePacket(PacketType::Heartbeat), START).has_value());
    cr_assert_not(table.isConnected(stranger));
    cr_assert_eq(table.getSize(), std::size_t{0});
}

Test(network_session_table, sweep_evicts_only_the_idle_peer)
{
    SessionTable table;
    const Endpoint active = Endpoint::loopback(40001);
    const Endpoint idle = Endpoint::loopback(40002);

    (void)table.onPacket(active, makePacket(PacketType::Connect), START);
    (void)table.onPacket(idle, makePacket(PacketType::Connect), START);
    (void)table.onPacket(active, makePacket(PacketType::Heartbeat), START + std::chrono::milliseconds{2500});

    const std::vector<Endpoint> gone = table.sweep(START + std::chrono::milliseconds{3000});

    cr_assert_eq(gone.size(), std::size_t{1});
    cr_assert(gone[0] == idle);
    cr_assert(table.isConnected(active));
    cr_assert_not(table.isConnected(idle));
}

Test(network_session_table, client_and_table_complete_a_handshake)
{
    Session session;
    SessionTable table;
    const Endpoint server = Endpoint::loopback(4242);
    const std::optional<SendRequest> connect = session.connect(server, START);

    cr_assert(connect.has_value());

    const Packet hello = decodeOrFail(connect->bytes);
    const std::optional<SendRequest> answer = table.onPacket(connect->to, hello, START);

    cr_assert(answer.has_value());
    session.onPacket(server, decodeOrFail(answer->bytes), START);
    cr_assert_eq(session.getState(), SessionState::Connected);
    cr_assert(table.isConnected(connect->to));
}

Test(network_session_table, game_packets_share_the_sequence_of_the_peer)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);
    const std::optional<SendRequest> ack = table.onPacket(peer, makePacket(PacketType::Connect), START);
    const std::optional<std::uint32_t> game = table.nextSequence(peer);
    const std::optional<SendRequest> heartbeat = table.onPacket(peer, makePacket(PacketType::Heartbeat), START);

    cr_assert(game.has_value());
    cr_assert_eq(*game, decodeOrFail(ack->bytes).seq + 1);
    cr_assert_eq(decodeOrFail(heartbeat->bytes).seq, *game + 1);
    cr_assert_not(table.nextSequence(Endpoint::loopback(40002)).has_value());
}

Test(network_session_table, a_client_counts_no_loss_among_several_peers)
{
    MetricsStore metrics;
    Session session(metrics);
    SessionTable table;
    const Endpoint server = Endpoint::loopback(4242);
    const Endpoint client = Endpoint::loopback(40001);
    const Endpoint other = Endpoint::loopback(40002);

    (void)session.connect(server, START);
    (void)table.onPacket(other, makePacket(PacketType::Connect), START);
    session.onPacket(server, decodeOrFail(table.onPacket(client, makePacket(PacketType::Connect), START)->bytes), START);
    // The server numbers a packet for each peer: the packets of the other one leave no hole for this client
    for (int step = 0; step < 3; ++step) {
        (void)table.nextSequence(other);
        session.onPacket(server, makePacket(PacketType::GameData, *table.nextSequence(client)), START);
    }
    cr_assert_not(metrics.getMetric("network.packets_lost").has_value());
}

Test(network_session_table, lost_answer_is_covered_by_a_second_connect)
{
    Session session;
    SessionTable table;
    const Endpoint server = Endpoint::loopback(4242);
    const Endpoint client = Endpoint::loopback(40001);
    const std::chrono::steady_clock::time_point later = START + std::chrono::milliseconds{500};
    const std::optional<SendRequest> connect = session.connect(server, START);

    cr_assert(connect.has_value());
    // The first answer is lost on its way back to the client
    (void)table.onPacket(client, decodeOrFail(connect->bytes), START);

    const std::optional<SendRequest> again = session.pollHeartbeat(later);

    cr_assert(again.has_value());

    const std::optional<SendRequest> answer = table.onPacket(client, decodeOrFail(again->bytes), later);

    cr_assert(answer.has_value());
    session.onPacket(server, decodeOrFail(answer->bytes), later);
    cr_assert_eq(session.getState(), SessionState::Connected);
    cr_assert_eq(table.getSize(), std::size_t{1});
}

Test(network_session_table, heartbeats_alone_keep_both_sides_connected)
{
    Session session;
    SessionTable table;
    const Endpoint server = Endpoint::loopback(4242);
    const Endpoint client = Endpoint::loopback(40001);
    const std::optional<SendRequest> connect = session.connect(server, START);

    cr_assert(connect.has_value());

    const std::optional<SendRequest> ack = table.onPacket(client, decodeOrFail(connect->bytes), START);

    cr_assert(ack.has_value());
    session.onPacket(server, decodeOrFail(ack->bytes), START);
    // Ten seconds, far past the timeout, with heartbeats as the only traffic
    for (int elapsed = 100; elapsed <= 10000; elapsed += 100) {
        const std::chrono::steady_clock::time_point now = START + std::chrono::milliseconds{elapsed};
        const std::optional<SendRequest> heartbeat = session.pollHeartbeat(now);

        if (heartbeat.has_value()) {
            const std::optional<SendRequest> answer = table.onPacket(client, decodeOrFail(heartbeat->bytes), now);

            cr_assert(answer.has_value());
            session.onPacket(server, decodeOrFail(answer->bytes), now);
        }
        cr_assert_eq(session.update(now), SessionState::Connected);
        cr_assert(table.sweep(now).empty());
    }
    cr_assert(table.isConnected(client));
}

Test(network_session_table, ping_is_answered_by_a_pong_with_the_same_cookie)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);
    const std::vector<std::byte> cookie = makeCookie();

    (void)table.onPacket(peer, makePacket(PacketType::Connect), START);
    const std::optional<SendRequest> answer =
        table.onPacket(peer, makePing(cookie), START + std::chrono::milliseconds{10});

    cr_assert(answer.has_value());
    cr_assert(answer->to == peer);
    const Packet pong = decodeOrFail(answer->bytes);

    cr_assert_eq(pong.type, PacketType::Pong);
    cr_assert(pong.payload == cookie);
}

Test(network_session_table, ping_refreshes_the_liveness)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);

    (void)table.onPacket(peer, makePacket(PacketType::Connect), START);
    (void)table.onPacket(peer, makePing(makeCookie()), START + std::chrono::milliseconds{2500});
    cr_assert(table.sweep(START + std::chrono::milliseconds{5499}).empty());
    cr_assert_eq(table.sweep(START + std::chrono::milliseconds{5500}).size(), std::size_t{1});
}

Test(network_session_table, answers_carry_increasing_sequence_numbers)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);

    const std::optional<SendRequest> ack = table.onPacket(peer, makePacket(PacketType::Connect), START);
    const std::optional<SendRequest> heartbeat =
        table.onPacket(peer, makePacket(PacketType::Heartbeat), START + std::chrono::milliseconds{10});
    const std::optional<SendRequest> pong =
        table.onPacket(peer, makePing(makeCookie()), START + std::chrono::milliseconds{20});

    cr_assert(ack.has_value());
    cr_assert(heartbeat.has_value());
    cr_assert(pong.has_value());
    cr_assert_eq(decodeOrFail(ack->bytes).seq, 0U);
    cr_assert_eq(decodeOrFail(heartbeat->bytes).seq, 1U);
    cr_assert_eq(decodeOrFail(pong->bytes).seq, 2U);
}

Test(network_session_table, reconnect_restarts_the_sequence_numbers)
{
    SessionTable table;
    const Endpoint peer = Endpoint::loopback(40001);

    const std::optional<SendRequest> first = table.onPacket(peer, makePacket(PacketType::Connect), START);
    const std::optional<SendRequest> heartbeat =
        table.onPacket(peer, makePacket(PacketType::Heartbeat), START + std::chrono::milliseconds{10});
    const std::optional<SendRequest> second =
        table.onPacket(peer, makePacket(PacketType::Connect), START + std::chrono::milliseconds{20});

    cr_assert(first.has_value());
    cr_assert(heartbeat.has_value());
    cr_assert(second.has_value());
    cr_assert_eq(decodeOrFail(first->bytes).seq, 0U);
    cr_assert_eq(decodeOrFail(heartbeat->bytes).seq, 1U);
    cr_assert_eq(decodeOrFail(second->bytes).seq, 0U);
}

Test(network_session_table, ping_from_a_stranger_is_dropped)
{
    SessionTable table;
    const Endpoint stranger = Endpoint::loopback(40002);

    cr_assert_not(table.onPacket(stranger, makePing(makeCookie()), START).has_value());
    cr_assert_eq(table.getSize(), std::size_t{0});
}

Test(network_session_table, client_measures_the_latency_through_the_table)
{
    MetricsStore metrics;
    Session session(
        SessionConfig{
            .heartbeatInterval = std::chrono::milliseconds{5000},
            .timeout = std::chrono::milliseconds{10000},
            .pingInterval = std::chrono::milliseconds{1000}},
        metrics);
    SessionTable table;
    const Endpoint server = Endpoint::loopback(4242);
    const Endpoint client = Endpoint::loopback(40001);
    const std::chrono::steady_clock::time_point pingAt = START + std::chrono::milliseconds{1000};
    const std::chrono::steady_clock::time_point pongAt = START + std::chrono::milliseconds{1100};

    const std::optional<SendRequest> connect = session.connect(server, START);

    cr_assert(connect.has_value());

    const std::optional<SendRequest> ack = table.onPacket(client, decodeOrFail(connect->bytes), START);

    cr_assert(ack.has_value());
    session.onPacket(server, decodeOrFail(ack->bytes), START);

    const std::optional<SendRequest> ping = session.pollHeartbeat(pingAt);

    cr_assert(ping.has_value());

    const std::optional<SendRequest> pong = table.onPacket(client, decodeOrFail(ping->bytes), pingAt);

    cr_assert(pong.has_value());
    session.onPacket(server, decodeOrFail(pong->bytes), pongAt);

    const std::optional<younity::tools::Metric> latency = metrics.getMetric("network.latency");

    cr_assert(latency.has_value());
    cr_assert_eq(latency->kind, younity::tools::MetricKind::Timer);
    cr_assert_float_eq(latency->last, 0.1, 0.000001);
    cr_assert_not(metrics.getMetric("network.packets_lost").has_value());
}

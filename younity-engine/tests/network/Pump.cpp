/*
** EPITECH PROJECT, 2026
** younity-engine/tests/network/Pump
** File description:
** UdpPump unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <thread>
#include <vector>

#include "younity/network/Datagram.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Packet.hpp"
#include "younity/network/SocketError.hpp"
#include "younity/network/UdpPump.hpp"
#include "younity/network/UdpSocket.hpp"
#include "younity/network/Writer.hpp"

using younity::network::Endpoint;
using younity::network::Packet;
using younity::network::PacketType;
using younity::network::ReceivedDatagram;
using younity::network::SocketError;
using younity::network::UdpPump;
using younity::network::UdpSocket;

namespace {

std::optional<ReceivedDatagram> pollReceive(UdpPump &pump)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);

    while (std::chrono::steady_clock::now() < deadline) {
        std::optional<ReceivedDatagram> datagram = pump.tryReceive();

        if (datagram.has_value()) {
            return datagram;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return std::nullopt;
}

} // namespace

Test(network_pump, starts_closed)
{
    const UdpPump pump;

    cr_assert_not(pump.isOpen());
    cr_assert_not(pump.isRunning());
    cr_assert_eq(pump.getLocalEndpoint().error(), SocketError::NotOpen);
}

Test(network_pump, start_without_bind_fails)
{
    UdpPump pump;

    cr_assert_eq(pump.start().error(), SocketError::NotOpen);
    cr_assert_not(pump.isRunning());
}

Test(network_pump, bind_and_start_then_stop_closes)
{
    UdpPump pump;

    cr_assert(pump.bind(0).has_value());
    cr_assert(pump.isOpen());
    cr_assert(pump.start().has_value());
    cr_assert(pump.isRunning());
    pump.stop();
    cr_assert_not(pump.isRunning());
    cr_assert_not(pump.isOpen());
}

Test(network_pump, start_is_idempotent)
{
    UdpPump pump;

    cr_assert(pump.bind(Endpoint::loopback(0)).has_value());
    cr_assert(pump.start().has_value());
    cr_assert(pump.start().has_value());
    cr_assert(pump.isRunning());
    pump.stop();
}

Test(network_pump, stop_without_start_is_safe)
{
    UdpPump pump;

    pump.stop();
    pump.stop();
    cr_assert_not(pump.isRunning());
}

Test(network_pump, bind_while_running_is_refused)
{
    UdpPump pump;

    cr_assert(pump.bind(0).has_value());
    cr_assert(pump.start().has_value());
    cr_assert_eq(pump.bind(0).error(), SocketError::BindFailed);
    cr_assert(pump.isRunning());
    pump.stop();
}

Test(network_pump, restart_needs_a_new_bind)
{
    UdpPump pump;

    cr_assert(pump.bind(0).has_value());
    cr_assert(pump.start().has_value());
    pump.stop();
    cr_assert_eq(pump.start().error(), SocketError::NotOpen);
    cr_assert(pump.bind(0).has_value());
    cr_assert(pump.start().has_value());
    pump.stop();
}

Test(network_pump, packet_survives_pump_loopback)
{
    UdpPump sender;
    UdpPump receiver;

    cr_assert(sender.bind(Endpoint::loopback(0)).has_value());
    cr_assert(receiver.bind(Endpoint::loopback(0)).has_value());
    cr_assert(sender.start().has_value());
    cr_assert(receiver.start().has_value());

    const std::expected<Endpoint, SocketError> senderLocal = sender.getLocalEndpoint();
    const std::expected<Endpoint, SocketError> receiverLocal = receiver.getLocalEndpoint();

    cr_assert(senderLocal.has_value());
    cr_assert(receiverLocal.has_value());

    const std::vector<std::byte> payload{std::byte{0xDE}, std::byte{0xAD}};
    const Packet sent{.type = PacketType::GameData, .seq = 7, .payload = payload};
    younity::network::Writer wire;

    cr_assert(younity::network::encodePacket(sent, wire));
    cr_assert(sender.sendTo(Endpoint::loopback(receiverLocal->getPort()), wire.getData()));

    const std::optional<ReceivedDatagram> datagram = pollReceive(receiver);

    cr_assert(datagram.has_value());
    cr_assert_eq(datagram->from.getPort(), senderLocal->getPort());

    const std::expected<Packet, younity::network::PacketError> received =
        younity::network::decodePacket(datagram->bytes);

    cr_assert(received.has_value());
    cr_assert_eq(received->type, PacketType::GameData);
    cr_assert_eq(received->seq, std::uint32_t{7});
    cr_assert_eq(received->payload, payload);
    sender.stop();
    receiver.stop();
}

Test(network_pump, wait_receive_gives_the_next_datagram)
{
    UdpPump sender;
    UdpPump receiver;

    cr_assert(sender.bind(Endpoint::loopback(0)).has_value());
    cr_assert(receiver.bind(Endpoint::loopback(0)).has_value());
    cr_assert(sender.start().has_value());
    cr_assert(receiver.start().has_value());

    const std::vector<std::byte> payload{std::byte{'w'}};

    cr_assert(sender.sendTo(Endpoint::loopback(receiver.getLocalEndpoint()->getPort()), payload));

    const std::optional<ReceivedDatagram> datagram = receiver.waitReceive();

    cr_assert(datagram.has_value());
    cr_assert_eq(datagram->bytes, payload);
    sender.stop();
    receiver.stop();
}

Test(network_pump, answers_both_ways)
{
    UdpPump first;
    UdpPump second;

    cr_assert(first.bind(Endpoint::loopback(0)).has_value());
    cr_assert(second.bind(Endpoint::loopback(0)).has_value());
    cr_assert(first.start().has_value());
    cr_assert(second.start().has_value());

    const std::expected<Endpoint, SocketError> firstLocal = first.getLocalEndpoint();
    const std::expected<Endpoint, SocketError> secondLocal = second.getLocalEndpoint();

    cr_assert(firstLocal.has_value());
    cr_assert(secondLocal.has_value());

    const std::vector<std::byte> ping{std::byte{'p'}};
    const std::vector<std::byte> pong{std::byte{'q'}};

    cr_assert(first.sendTo(Endpoint::loopback(secondLocal->getPort()), ping));
    cr_assert(second.sendTo(Endpoint::loopback(firstLocal->getPort()), pong));
    cr_assert_eq(pollReceive(first)->bytes, pong);
    cr_assert_eq(pollReceive(second)->bytes, ping);
    first.stop();
    second.stop();
}

Test(network_pump, send_after_stop_returns_false)
{
    UdpPump pump;

    cr_assert(pump.bind(0).has_value());
    cr_assert(pump.start().has_value());
    pump.stop();

    const std::vector<std::byte> payload{std::byte{'h'}, std::byte{'i'}};

    cr_assert_not(pump.sendTo(Endpoint::loopback(9), payload));
}

Test(network_pump, oversized_send_is_refused)
{
    UdpPump pump;

    cr_assert(pump.bind(0).has_value());
    cr_assert(pump.start().has_value());

    const std::vector<std::byte> huge(younity::network::MAX_PACKET_BYTES + 1);

    cr_assert_not(pump.sendTo(Endpoint::loopback(9), huge));
    cr_assert_eq(pump.getOutgoingSize(), std::size_t{0});
    pump.stop();
}

Test(network_pump, destructor_joins_the_threads)
{
    {
        UdpPump pump;

        cr_assert(pump.bind(0).has_value());
        cr_assert(pump.start().has_value());
        cr_assert(pump.isRunning());
    }
    cr_assert(true);
}

Test(network_pump, drops_a_datagram_larger_than_a_packet)
{
    UdpSocket sender;
    UdpPump receiver;

    cr_assert(sender.bind(Endpoint::loopback(0)).has_value());
    cr_assert(receiver.bind(Endpoint::loopback(0)).has_value());
    cr_assert(receiver.start().has_value());

    const Endpoint target = Endpoint::loopback(receiver.getLocalEndpoint()->getPort());
    const std::vector<std::byte> tooLarge(younity::network::MAX_PACKET_BYTES + 1, std::byte{'x'});
    const std::vector<std::byte> packet{std::byte{'y'}};

    cr_assert(sender.sendTo(tooLarge, target).has_value());
    cr_assert(sender.sendTo(packet, target).has_value());

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);

    while (receiver.getIncomingSize() == 0 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const std::optional<ReceivedDatagram> datagram = receiver.tryReceive();

    cr_assert(datagram.has_value());
    cr_assert_eq(datagram->bytes, packet);
    cr_assert_eq(receiver.getDroppedIncoming(), std::size_t{1});
}

Test(network_pump, incoming_stays_readable_after_stop)
{
    UdpPump sender;
    UdpPump receiver;

    cr_assert(sender.bind(Endpoint::loopback(0)).has_value());
    cr_assert(receiver.bind(Endpoint::loopback(0)).has_value());
    cr_assert(sender.start().has_value());
    cr_assert(receiver.start().has_value());

    const std::expected<Endpoint, SocketError> receiverLocal = receiver.getLocalEndpoint();

    cr_assert(receiverLocal.has_value());

    const std::vector<std::byte> payload{std::byte{'x'}};

    cr_assert(sender.sendTo(Endpoint::loopback(receiverLocal->getPort()), payload));

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);

    while (receiver.getIncomingSize() == 0 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    cr_assert_gt(receiver.getIncomingSize(), std::size_t{0});
    receiver.stop();
    sender.stop();

    const std::optional<ReceivedDatagram> datagram = receiver.tryReceive();

    cr_assert(datagram.has_value());
    cr_assert_eq(datagram->bytes, payload);
}

Test(network_pump, incoming_queue_is_bounded_and_counts_drops)
{
    UdpPump sender;
    UdpPump receiver(1);

    cr_assert(sender.bind(Endpoint::loopback(0)).has_value());
    cr_assert(receiver.bind(Endpoint::loopback(0)).has_value());
    cr_assert(sender.start().has_value());
    cr_assert(receiver.start().has_value());

    const std::expected<Endpoint, SocketError> receiverLocal = receiver.getLocalEndpoint();

    cr_assert(receiverLocal.has_value());

    const Endpoint target = Endpoint::loopback(receiverLocal->getPort());
    const std::vector<std::byte> first{std::byte{0x01}};
    const std::vector<std::byte> second{std::byte{0x02}};

    cr_assert(sender.sendTo(target, first));

    const auto queued = std::chrono::steady_clock::now() + std::chrono::seconds(2);

    while (receiver.getIncomingSize() == 0 && std::chrono::steady_clock::now() < queued) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    cr_assert_eq(receiver.getIncomingSize(), std::size_t{1});

    cr_assert(sender.sendTo(target, second));

    const auto dropped = std::chrono::steady_clock::now() + std::chrono::seconds(2);

    while (receiver.getDroppedIncoming() == 0 && std::chrono::steady_clock::now() < dropped) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    cr_assert_eq(receiver.getDroppedIncoming(), std::size_t{1});
    cr_assert_eq(receiver.getIncomingSize(), std::size_t{1});
    receiver.stop();
    sender.stop();
}

Test(network_pump, zero_capacity_refuses_every_send)
{
    UdpPump pump(0);
    const std::vector<std::byte> payload{std::byte{'x'}};

    cr_assert_not(pump.sendTo(Endpoint::loopback(9), payload));
    cr_assert_eq(pump.getOutgoingSize(), std::size_t{0});
}

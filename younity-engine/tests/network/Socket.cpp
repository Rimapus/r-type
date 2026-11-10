/*
** EPITECH PROJECT, 2026
** younity-engine/tests/network/Socket
** File description:
** Socket unit tests
*/

#include <criterion/criterion.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "younity/network/Endpoint.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Reader.hpp"
#include "younity/network/SocketError.hpp"
#include "younity/network/UdpSocket.hpp"
#include "younity/network/Writer.hpp"

using younity::network::Endpoint;
using younity::network::SocketError;
using younity::network::UdpSocket;

Test(network_socket_error, describes_all_errors)
{
    cr_assert_eq(younity::network::toString(SocketError::NotOpen), "not open");
    cr_assert_eq(younity::network::toString(SocketError::OpenFailed), "open failed");
    cr_assert_eq(younity::network::toString(SocketError::BindFailed), "bind failed");
    cr_assert_eq(younity::network::toString(SocketError::SendFailed), "send failed");
    cr_assert_eq(younity::network::toString(SocketError::ReceiveFailed), "receive failed");
    cr_assert_eq(younity::network::toString(SocketError::InvalidAddress), "invalid address");
    cr_assert_eq(younity::network::toString(SocketError::TooLarge), "too large");
}

Test(network_endpoint, loopback_and_text)
{
    const Endpoint endpoint = Endpoint::loopback(40001);

    cr_assert_eq(endpoint.getAddress(), "127.0.0.1");
    cr_assert_eq(endpoint.getPort(), std::uint16_t{40001});
    cr_assert_eq(endpoint.toString(), "127.0.0.1:40001");
    cr_assert_eq(endpoint, Endpoint("127.0.0.1", 40001));
}

Test(network_endpoint, resolve_gives_the_address_of_a_host)
{
    const std::expected<Endpoint, SocketError> local = Endpoint::resolve("localhost", 4242);
    const std::expected<Endpoint, SocketError> numeric = Endpoint::resolve("192.168.1.10", 4243);

    cr_assert(local.has_value());
    cr_assert(*local == Endpoint::loopback(4242));
    cr_assert(numeric.has_value());
    cr_assert(*numeric == Endpoint("192.168.1.10", 4243));
}

Test(network_endpoint, resolve_refuses_a_host_without_address)
{
    const std::expected<Endpoint, SocketError> missing = Endpoint::resolve("", 4242);

    cr_assert_not(missing.has_value());
    cr_assert_eq(missing.error(), SocketError::InvalidAddress);
}

Test(network_socket, starts_closed)
{
    const UdpSocket socket;

    cr_assert_not(socket.isOpen());
    cr_assert_eq(socket.getLocalEndpoint().error(), SocketError::NotOpen);
}

Test(network_socket, receive_on_unbound_reports_not_open)
{
    UdpSocket socket;
    std::vector<std::byte> buffer(younity::network::MAX_PACKET_BYTES);
    Endpoint sender = Endpoint::loopback(0);

    cr_assert_eq(socket.receiveFrom(buffer, sender).error(), SocketError::NotOpen);
}

Test(network_socket, invalid_address_is_rejected)
{
    UdpSocket socket;

    cr_assert(socket.bind(0).has_value());

    const std::vector<std::byte> payload{std::byte{'h'}, std::byte{'i'}};
    const std::expected<std::size_t, SocketError> sent = socket.sendTo(payload, Endpoint("999.999.999.999", 9));

    cr_assert_eq(sent.error(), SocketError::InvalidAddress);
}

Test(network_socket, bind_conflict_reports_an_error)
{
    UdpSocket first;

    cr_assert(first.bind(0).has_value());
    const std::expected<Endpoint, SocketError> local = first.getLocalEndpoint();

    cr_assert(local.has_value());

    UdpSocket second;
    const std::expected<void, SocketError> bound = second.bind(local->getPort());

    cr_assert_eq(bound.error(), SocketError::BindFailed);
}

Test(network_socket, bind_to_loopback_listens_on_localhost)
{
    UdpSocket socket;

    cr_assert(socket.bind(Endpoint::loopback(0)).has_value());

    const std::expected<Endpoint, SocketError> local = socket.getLocalEndpoint();

    cr_assert(local.has_value());
    cr_assert_eq(local->getAddress(), "127.0.0.1");
    cr_assert_neq(local->getPort(), std::uint16_t{0});
}

Test(network_socket, bind_to_invalid_address_is_rejected)
{
    UdpSocket socket;

    cr_assert_eq(socket.bind(Endpoint("999.999.999.999", 0)).error(), SocketError::InvalidAddress);
    cr_assert_not(socket.isOpen());
}

Test(network_socket, moved_socket_keeps_the_binding)
{
    UdpSocket socket;

    cr_assert(socket.bind(0).has_value());
    cr_assert(socket.isOpen());

    UdpSocket moved = std::move(socket);

    cr_assert(moved.isOpen());
    cr_assert(moved.getLocalEndpoint().has_value());
}

Test(network_socket, send_and_receive_loopback)
{
    UdpSocket sender;
    UdpSocket receiver;

    cr_assert(sender.bind(0).has_value());
    cr_assert(receiver.bind(0).has_value());

    const std::expected<Endpoint, SocketError> senderLocal = sender.getLocalEndpoint();
    const std::expected<Endpoint, SocketError> receiverLocal = receiver.getLocalEndpoint();

    cr_assert(senderLocal.has_value());
    cr_assert(receiverLocal.has_value());

    const std::vector<std::byte> payload{std::byte{'h'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'},
                                         std::byte{'o'}};
    const std::expected<std::size_t, SocketError> sent =
        sender.sendTo(payload, Endpoint::loopback(receiverLocal->getPort()));

    cr_assert(sent.has_value());
    cr_assert_eq(*sent, payload.size());

    std::vector<std::byte> buffer(younity::network::MAX_PACKET_BYTES);
    Endpoint from = Endpoint::loopback(0);
    const std::expected<std::size_t, SocketError> received = receiver.receiveFrom(buffer, from);

    cr_assert(received.has_value());
    cr_assert_eq(*received, payload.size());
    cr_assert_eq(from.getPort(), senderLocal->getPort());
    cr_assert_eq(std::vector<std::byte>(buffer.begin(), buffer.begin() + *received), payload);
}

Test(network_socket, serializer_payload_survives_loopback)
{
    younity::network::Writer writer;

    cr_assert(writer.putU32Le(0x12345678));
    cr_assert(writer.putString("enemies"));

    UdpSocket sender;
    UdpSocket receiver;

    cr_assert(sender.bind(0).has_value());
    cr_assert(receiver.bind(0).has_value());

    const std::expected<Endpoint, SocketError> receiverLocal = receiver.getLocalEndpoint();

    cr_assert(receiverLocal.has_value());
    cr_assert(sender.sendTo(writer.getData(), Endpoint::loopback(receiverLocal->getPort())).has_value());

    std::vector<std::byte> buffer(younity::network::MAX_PACKET_BYTES);
    Endpoint from = Endpoint::loopback(0);

    const std::expected<std::size_t, SocketError> received = receiver.receiveFrom(buffer, from);

    cr_assert(received.has_value());

    younity::network::Reader reader(std::span<const std::byte>(buffer.data(), *received));

    cr_assert_eq(*reader.getU32Le(), std::uint32_t{0x12345678});
    cr_assert_eq(*reader.getString(), "enemies");
}

Test(network_socket, move_assign_keeps_the_binding)
{
    UdpSocket socket;

    cr_assert(socket.bind(0).has_value());

    UdpSocket other;

    cr_assert_not(other.isOpen());
    other = std::move(socket);
    cr_assert(other.isOpen());
    cr_assert(other.getLocalEndpoint().has_value());
    cr_assert_not(socket.isOpen());
}

Test(network_socket, close_makes_the_socket_unusable)
{
    UdpSocket socket;

    cr_assert(socket.bind(0).has_value());
    socket.close();
    cr_assert_not(socket.isOpen());
    cr_assert_eq(socket.getLocalEndpoint().error(), SocketError::NotOpen);

    UdpSocket moved = std::move(socket);

    moved.close();
    socket.close();
}

Test(network_socket, oversized_datagram_fails_to_send)
{
    UdpSocket sender;
    UdpSocket receiver;

    cr_assert(receiver.bind(0).has_value());

    const std::expected<Endpoint, SocketError> receiverLocal = receiver.getLocalEndpoint();

    cr_assert(receiverLocal.has_value());

    const std::vector<std::byte> huge(70000);

    cr_assert_eq(sender.sendTo(huge, Endpoint::loopback(receiverLocal->getPort())).error(), SocketError::SendFailed);
    cr_assert(sender.isOpen());
}

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/network/Packet
** File description:
** Packet unit tests
*/

#include <criterion/criterion.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

#include "younity/network/Endpoint.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Packet.hpp"
#include "younity/network/PacketError.hpp"
#include "younity/network/PacketType.hpp"
#include "younity/network/Reader.hpp"
#include "younity/network/UdpSocket.hpp"
#include "younity/network/Writer.hpp"

using younity::network::Endpoint;
using younity::network::Packet;
using younity::network::PacketError;
using younity::network::PacketType;
using younity::network::Reader;
using younity::network::UdpSocket;
using younity::network::Writer;

namespace {

std::vector<std::byte> encodeOrFail(const Packet &packet)
{
    Writer writer;

    cr_assert(younity::network::encodePacket(packet, writer));
    const std::span<const std::byte> data = writer.getData();
    return std::vector<std::byte>(data.begin(), data.end());
}

Writer makePingWire(std::uint32_t seq, std::span<const std::byte> cookie)
{
    Writer writer;

    cr_assert(writer.putU16Le(younity::network::PROTOCOL_MAGIC));
    cr_assert(writer.putU8(younity::network::PROTOCOL_VERSION));
    cr_assert(writer.putU8(static_cast<std::uint8_t>(PacketType::Ping)));
    cr_assert(writer.putU32Le(seq));
    if (!cookie.empty()) {
        cr_assert(writer.putBytes(cookie));
    }
    return writer;
}

} // namespace

Test(network_packet_error, describes_all_errors)
{
    cr_assert_eq(younity::network::toString(PacketError::Truncated), std::string_view("truncated"));
    cr_assert_eq(younity::network::toString(PacketError::TooLarge), std::string_view("too large"));
    cr_assert_eq(younity::network::toString(PacketError::BadMagic), std::string_view("bad magic"));
    cr_assert_eq(younity::network::toString(PacketError::BadVersion), std::string_view("bad version"));
    cr_assert_eq(younity::network::toString(PacketError::UnknownType), std::string_view("unknown type"));
    cr_assert_eq(younity::network::toString(PacketError::InvalidPayload), std::string_view("invalid payload"));
}

Test(network_packet_error, unknown_value_falls_back)
{
    cr_assert_eq(younity::network::toString(static_cast<PacketError>(0xFF)), std::string_view("unknown"));
}

Test(network_packet_type, describes_all_types)
{
    cr_assert_eq(younity::network::toString(PacketType::GameData), std::string_view("game data"));
    cr_assert_eq(younity::network::toString(PacketType::Ping), std::string_view("ping"));
    cr_assert_eq(younity::network::toString(PacketType::Pong), std::string_view("pong"));
    cr_assert_eq(younity::network::toString(PacketType::Connect), std::string_view("connect"));
    cr_assert_eq(younity::network::toString(PacketType::ConnectAck), std::string_view("connect ack"));
    cr_assert_eq(younity::network::toString(PacketType::Heartbeat), std::string_view("heartbeat"));
}

Test(network_packet_type, unknown_value_falls_back)
{
    cr_assert_eq(younity::network::toString(static_cast<PacketType>(0xFF)), std::string_view("unknown"));
}

Test(network_packet_type, from_u8_accepts_defined_types_only)
{
    cr_assert_eq(*younity::network::packetTypeFromU8(0x01), PacketType::GameData);
    cr_assert_eq(*younity::network::packetTypeFromU8(0x02), PacketType::Ping);
    cr_assert_eq(*younity::network::packetTypeFromU8(0x03), PacketType::Pong);
    cr_assert_eq(*younity::network::packetTypeFromU8(0x80), PacketType::Connect);
    cr_assert_eq(*younity::network::packetTypeFromU8(0x81), PacketType::ConnectAck);
    cr_assert_eq(*younity::network::packetTypeFromU8(0x82), PacketType::Heartbeat);
    cr_assert_eq(younity::network::packetTypeFromU8(0x00).error(), PacketError::UnknownType);
    cr_assert_eq(younity::network::packetTypeFromU8(0x04).error(), PacketError::UnknownType);
    cr_assert_eq(younity::network::packetTypeFromU8(0x83).error(), PacketError::UnknownType);
    cr_assert_eq(younity::network::packetTypeFromU8(0xFF).error(), PacketError::UnknownType);
}

Test(network_packet, header_constants_match_the_plan)
{
    cr_assert_eq(younity::network::PROTOCOL_MAGIC, std::uint16_t{0x3659});
    cr_assert_eq(younity::network::PROTOCOL_VERSION, std::uint8_t{1});
    cr_assert_eq(younity::network::PACKET_HEADER_BYTES, std::size_t{8});
    cr_assert_eq(younity::network::PING_COOKIE_BYTES, std::size_t{4});
}

Test(network_packet, wire_layout_is_little_endian)
{
    const Packet packet{.type = PacketType::GameData, .seq = 0x12345678, .payload = {}};
    const std::vector<std::byte> raw = encodeOrFail(packet);

    cr_assert_eq(raw.size(), std::size_t{8});
    cr_assert_eq(static_cast<char>(raw[0]), 'Y');
    cr_assert_eq(static_cast<char>(raw[1]), '6');
    cr_assert_eq(static_cast<std::uint8_t>(raw[2]), std::uint8_t{1});
    cr_assert_eq(static_cast<std::uint8_t>(raw[3]), std::uint8_t{0x01});
    cr_assert_eq(static_cast<std::uint8_t>(raw[4]), std::uint8_t{0x78});
    cr_assert_eq(static_cast<std::uint8_t>(raw[5]), std::uint8_t{0x56});
    cr_assert_eq(static_cast<std::uint8_t>(raw[6]), std::uint8_t{0x34});
    cr_assert_eq(static_cast<std::uint8_t>(raw[7]), std::uint8_t{0x12});
}

Test(network_packet, gamedata_roundtrip_empty)
{
    const Packet sent{.type = PacketType::GameData, .seq = 7, .payload = {}};
    const std::vector<std::byte> raw = encodeOrFail(sent);
    const std::expected<Packet, PacketError> received = younity::network::decodePacket(raw);

    cr_assert(received.has_value());
    cr_assert_eq(received->type, PacketType::GameData);
    cr_assert_eq(received->seq, std::uint32_t{7});
    cr_assert(received->payload.empty());
}

Test(network_packet, gamedata_roundtrip_with_payload)
{
    const std::vector<std::byte> bytes{std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}};
    const Packet sent{.type = PacketType::GameData, .seq = 42, .payload = bytes};
    const std::vector<std::byte> raw = encodeOrFail(sent);
    const std::expected<Packet, PacketError> received = younity::network::decodePacket(raw);

    cr_assert(received.has_value());
    cr_assert_eq(received->type, PacketType::GameData);
    cr_assert_eq(received->seq, std::uint32_t{42});
    cr_assert_eq(received->payload, bytes);
}

Test(network_packet, ping_roundtrip_carries_the_cookie)
{
    Writer cookieWriter;

    cr_assert(cookieWriter.putU32Le(0xAABBCCDD));
    const std::span<const std::byte> cookie = cookieWriter.getData();
    const Packet sent{
        .type = PacketType::Ping, .seq = 99, .payload = std::vector<std::byte>(cookie.begin(), cookie.end())};
    const std::vector<std::byte> raw = encodeOrFail(sent);
    const std::expected<Packet, PacketError> received = younity::network::decodePacket(raw);

    cr_assert(received.has_value());
    cr_assert_eq(received->type, PacketType::Ping);
    cr_assert_eq(received->seq, std::uint32_t{99});

    Reader reader(received->payload);
    cr_assert_eq(*reader.getU32Le(), std::uint32_t{0xAABBCCDD});
    cr_assert_eq(reader.getRemaining(), std::size_t{0});
}

Test(network_packet, pong_roundtrip_carries_the_cookie)
{
    Writer cookieWriter;

    cr_assert(cookieWriter.putU32Le(0x01020304));
    const std::span<const std::byte> cookie = cookieWriter.getData();
    const Packet sent{
        .type = PacketType::Pong, .seq = 100, .payload = std::vector<std::byte>(cookie.begin(), cookie.end())};
    const std::vector<std::byte> raw = encodeOrFail(sent);
    const std::expected<Packet, PacketError> received = younity::network::decodePacket(raw);

    cr_assert(received.has_value());
    cr_assert_eq(received->type, PacketType::Pong);
    cr_assert_eq(received->seq, std::uint32_t{100});

    Reader reader(received->payload);
    cr_assert_eq(*reader.getU32Le(), std::uint32_t{0x01020304});
    cr_assert_eq(reader.getRemaining(), std::size_t{0});
}

Test(network_packet, sequence_number_wraps)
{
    const Packet sent{.type = PacketType::GameData, .seq = std::numeric_limits<std::uint32_t>::max(), .payload = {}};
    const std::vector<std::byte> raw = encodeOrFail(sent);
    const std::expected<Packet, PacketError> received = younity::network::decodePacket(raw);

    cr_assert(received.has_value());
    cr_assert_eq(received->seq, std::numeric_limits<std::uint32_t>::max());
}

Test(network_packet, empty_datagram_is_truncated)
{
    const std::vector<std::byte> empty;
    cr_assert_eq(younity::network::decodePacket(empty).error(), PacketError::Truncated);
}

Test(network_packet, short_header_is_truncated)
{
    const std::vector<std::byte> raw = encodeOrFail(Packet{});
    const std::span<const std::byte> shortSpan(raw.data(), younity::network::PACKET_HEADER_BYTES - 1);

    cr_assert_eq(younity::network::decodePacket(shortSpan).error(), PacketError::Truncated);
}

Test(network_packet, bad_magic_is_rejected)
{
    std::vector<std::byte> raw = encodeOrFail(Packet{});

    raw[0] = std::byte{0x00};
    raw[1] = std::byte{0x00};
    cr_assert_eq(younity::network::decodePacket(raw).error(), PacketError::BadMagic);
}

Test(network_packet, bad_version_is_rejected)
{
    std::vector<std::byte> raw = encodeOrFail(Packet{});

    raw[2] = std::byte{0xFF};
    cr_assert_eq(younity::network::decodePacket(raw).error(), PacketError::BadVersion);
}

Test(network_packet, unknown_type_is_rejected)
{
    std::vector<std::byte> raw = encodeOrFail(Packet{});

    raw[3] = std::byte{0xFF};
    cr_assert_eq(younity::network::decodePacket(raw).error(), PacketError::UnknownType);
}

Test(network_packet, oversized_datagram_is_rejected)
{
    const std::vector<std::byte> huge(younity::network::MAX_PACKET_BYTES + 1);
    cr_assert_eq(younity::network::decodePacket(huge).error(), PacketError::TooLarge);
}

Test(network_packet, ping_without_cookie_is_rejected)
{
    const Writer wire = makePingWire(1, {});

    cr_assert_eq(younity::network::decodePacket(wire.getData()).error(), PacketError::InvalidPayload);
}

Test(network_packet, ping_with_short_cookie_is_rejected)
{
    const std::vector<std::byte> three{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    const Writer wire = makePingWire(1, three);

    cr_assert_eq(younity::network::decodePacket(wire.getData()).error(), PacketError::InvalidPayload);
}

Test(network_packet, ping_with_trailing_byte_is_rejected)
{
    const std::vector<std::byte> five{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
    const Writer wire = makePingWire(1, five);

    cr_assert_eq(younity::network::decodePacket(wire.getData()).error(), PacketError::InvalidPayload);
}

Test(network_packet, encode_refuses_ping_with_bad_payload)
{
    Writer writer;
    const Packet bad{.type = PacketType::Ping, .seq = 1, .payload = {}};

    cr_assert_not(younity::network::encodePacket(bad, writer));
    cr_assert_eq(writer.getSize(), std::size_t{0});
}

Test(network_packet, encode_refuses_oversized_gamedata)
{
    Writer writer;
    const Packet huge{
        .type = PacketType::GameData, .seq = 1, .payload = std::vector<std::byte>(younity::network::MAX_PACKET_BYTES)};

    cr_assert_not(younity::network::encodePacket(huge, writer));
    cr_assert_eq(writer.getSize(), std::size_t{0});
}

Test(network_packet, encode_accepts_largest_gamedata)
{
    Writer writer;
    const Packet largest{.type = PacketType::GameData,
                         .seq = 1,
                         .payload = std::vector<std::byte>(younity::network::MAX_PACKET_BYTES - younity::network::PACKET_HEADER_BYTES)};

    cr_assert(younity::network::encodePacket(largest, writer));
    cr_assert_eq(writer.getSize(), younity::network::MAX_PACKET_BYTES);

    const std::expected<Packet, PacketError> received = younity::network::decodePacket(writer.getData());
    cr_assert(received.has_value());
    cr_assert_eq(received->payload.size(), younity::network::MAX_PACKET_BYTES - younity::network::PACKET_HEADER_BYTES);
}

Test(network_packet, pong_without_cookie_is_rejected)
{
    Writer wire;

    cr_assert(wire.putU16Le(younity::network::PROTOCOL_MAGIC));
    cr_assert(wire.putU8(younity::network::PROTOCOL_VERSION));
    cr_assert(wire.putU8(static_cast<std::uint8_t>(PacketType::Pong)));
    cr_assert(wire.putU32Le(1));
    cr_assert_eq(younity::network::decodePacket(wire.getData()).error(), PacketError::InvalidPayload);
}

Test(network_packet, pong_with_trailing_byte_is_rejected)
{
    Writer wire;
    const std::vector<std::byte> five{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};

    cr_assert(wire.putU16Le(younity::network::PROTOCOL_MAGIC));
    cr_assert(wire.putU8(younity::network::PROTOCOL_VERSION));
    cr_assert(wire.putU8(static_cast<std::uint8_t>(PacketType::Pong)));
    cr_assert(wire.putU32Le(1));
    cr_assert(wire.putBytes(five));
    cr_assert_eq(younity::network::decodePacket(wire.getData()).error(), PacketError::InvalidPayload);
}

Test(network_packet, encode_refuses_unknown_type)
{
    Writer writer;
    const Packet bad{.type = static_cast<PacketType>(0xFF), .seq = 1, .payload = {}};

    cr_assert_not(younity::network::encodePacket(bad, writer));
    cr_assert_eq(writer.getSize(), std::size_t{0});
}

Test(network_packet, encode_appends_after_existing_content)
{
    Writer writer;

    cr_assert(writer.putU8(0xAA));
    cr_assert(writer.putU8(0xBB));

    const Packet packet{.type = PacketType::GameData, .seq = 3, .payload = {}};
    cr_assert(younity::network::encodePacket(packet, writer));
    cr_assert_eq(writer.getSize(), std::size_t{2 + younity::network::PACKET_HEADER_BYTES});

    const std::span<const std::byte> suffix(writer.getData().data() + 2, younity::network::PACKET_HEADER_BYTES);
    const std::expected<Packet, PacketError> received = younity::network::decodePacket(suffix);

    cr_assert(received.has_value());
    cr_assert_eq(received->seq, std::uint32_t{3});
}

Test(network_packet, encode_refuses_without_touching_non_empty_writer)
{
    Writer writer;

    cr_assert(writer.putU8(0xAA));
    cr_assert_eq(writer.getSize(), std::size_t{1});

    const Packet huge{
        .type = PacketType::GameData, .seq = 1, .payload = std::vector<std::byte>(younity::network::MAX_PACKET_BYTES)};

    cr_assert_not(younity::network::encodePacket(huge, writer));
    cr_assert_eq(writer.getSize(), std::size_t{1});
}

Test(network_packet, packet_survives_loopback)
{
    Writer cookieWriter;

    cr_assert(cookieWriter.putU32Le(0x12345678));
    const std::span<const std::byte> cookie = cookieWriter.getData();
    const Packet sent{
        .type = PacketType::Ping, .seq = 2026, .payload = std::vector<std::byte>(cookie.begin(), cookie.end())};

    Writer wire;
    cr_assert(younity::network::encodePacket(sent, wire));

    UdpSocket sender;
    UdpSocket receiver;

    cr_assert(sender.bind(0).has_value());
    cr_assert(receiver.bind(0).has_value());

    const std::expected<Endpoint, younity::network::SocketError> receiverLocal = receiver.getLocalEndpoint();
    cr_assert(receiverLocal.has_value());
    cr_assert(sender.sendTo(wire.getData(), Endpoint::loopback(receiverLocal->getPort())).has_value());

    std::vector<std::byte> buffer(younity::network::MAX_PACKET_BYTES);
    Endpoint from = Endpoint::loopback(0);
    const std::expected<std::size_t, younity::network::SocketError> receivedBytes = receiver.receiveFrom(buffer, from);

    cr_assert(receivedBytes.has_value());

    const std::expected<Packet, PacketError> received =
        younity::network::decodePacket(std::span<const std::byte>(buffer.data(), *receivedBytes));

    cr_assert(received.has_value());
    cr_assert_eq(received->type, PacketType::Ping);
    cr_assert_eq(received->seq, std::uint32_t{2026});

    Reader reader(received->payload);
    cr_assert_eq(*reader.getU32Le(), std::uint32_t{0x12345678});
}

Test(network_packet, session_packets_roundtrip_empty)
{
    const PacketType types[] = {PacketType::Connect, PacketType::ConnectAck, PacketType::Heartbeat};

    for (const PacketType type : types) {
        const Packet sent{.type = type, .seq = 5, .payload = {}};
        const std::vector<std::byte> raw = encodeOrFail(sent);
        const std::expected<Packet, PacketError> received = younity::network::decodePacket(raw);

        cr_assert(received.has_value());
        cr_assert_eq(received->type, type);
        cr_assert_eq(received->seq, std::uint32_t{5});
        cr_assert(received->payload.empty());
    }
}

Test(network_packet, session_packet_with_payload_is_rejected)
{
    Writer wire;

    cr_assert(wire.putU16Le(younity::network::PROTOCOL_MAGIC));
    cr_assert(wire.putU8(younity::network::PROTOCOL_VERSION));
    cr_assert(wire.putU8(static_cast<std::uint8_t>(PacketType::Connect)));
    cr_assert(wire.putU32Le(1));
    cr_assert(wire.putU8(0xFF));
    cr_assert_eq(younity::network::decodePacket(wire.getData()).error(), PacketError::InvalidPayload);
}

Test(network_packet, encode_refuses_session_packet_with_payload)
{
    Writer writer;
    const Packet bad{.type = PacketType::Heartbeat, .seq = 1, .payload = {std::byte{0x01}}};

    cr_assert_not(younity::network::encodePacket(bad, writer));
    cr_assert_eq(writer.getSize(), std::size_t{0});
}

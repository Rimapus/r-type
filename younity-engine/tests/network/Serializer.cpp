/*
** EPITECH PROJECT, 2026
** younity-engine/tests/network/Serializer
** File description:
** Serializer unit tests
*/

#include <criterion/criterion.h>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "younity/network/CodecError.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Reader.hpp"
#include "younity/network/Writer.hpp"

using younity::network::CodecError;
using younity::network::Reader;
using younity::network::Writer;

Test(network_codec_error, describes_all_errors)
{
    cr_assert_eq(younity::network::toString(CodecError::Truncated), std::string_view("truncated"));
    cr_assert_eq(younity::network::toString(CodecError::TooLarge), std::string_view("too large"));
    cr_assert_eq(younity::network::toString(CodecError::InvalidValue), std::string_view("invalid value"));
}

Test(network_limits, caps_match_the_plan)
{
    cr_assert_eq(younity::network::MAX_PACKET_BYTES, std::size_t{1200});
    cr_assert_eq(younity::network::MAX_STRING_BYTES, std::size_t{256});
}

Test(network_writer, integers_are_little_endian_on_the_wire)
{
    Writer writer;

    cr_assert(writer.putU16Le(0x1234));
    cr_assert_eq(writer.getSize(), std::size_t{2});
    const std::span<const std::byte> data = writer.getData();

    cr_assert_eq(static_cast<std::uint8_t>(data[0]), std::uint8_t{0x34});
    cr_assert_eq(static_cast<std::uint8_t>(data[1]), std::uint8_t{0x12});
}

Test(network_serializer, integers_roundtrip)
{
    Writer writer;

    cr_assert(writer.putU8(0xAB));
    cr_assert(writer.putU16Le(0x1234));
    cr_assert(writer.putU32Le(0x89ABCDEF));
    cr_assert(writer.putU64Le(0x1122334455667788ULL));
    cr_assert(writer.putBool(true));
    cr_assert(writer.putBool(false));

    Reader reader(writer.getData());

    cr_assert_eq(*reader.getU8(), std::uint8_t{0xAB});
    cr_assert_eq(*reader.getU16Le(), std::uint16_t{0x1234});
    cr_assert_eq(*reader.getU32Le(), std::uint32_t{0x89ABCDEF});
    cr_assert_eq(*reader.getU64Le(), std::uint64_t{0x1122334455667788ULL});
    cr_assert_eq(*reader.getBool(), true);
    cr_assert_eq(*reader.getBool(), false);
    cr_assert_eq(reader.getRemaining(), std::size_t{0});
}

Test(network_serializer, float_roundtrip_is_bit_exact)
{
    Writer writer;

    cr_assert(writer.putFloatLe(1.5F));

    Reader reader(writer.getData());
    const std::expected<float, CodecError> value = reader.getFloatLe();

    cr_assert(value.has_value());
    cr_assert_eq(std::bit_cast<std::uint32_t>(*value), std::bit_cast<std::uint32_t>(1.5F));
}

Test(network_serializer, string_and_bytes_roundtrip)
{
    Writer writer;
    const std::vector<std::byte> blob{std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}};

    cr_assert(writer.putString("player"));
    cr_assert(writer.putBytes(blob));

    Reader reader(writer.getData());

    cr_assert_eq(*reader.getString(), "player");
    cr_assert_eq(*reader.getBytes(blob.size()), blob);
    cr_assert_eq(reader.getRemaining(), std::size_t{0});
}

Test(network_serializer, empty_string_roundtrip)
{
    Writer writer;

    cr_assert(writer.putString(""));

    Reader reader(writer.getData());

    cr_assert_eq(*reader.getString(), "");
}

Test(network_reader, truncated_inputs_report_truncated)
{
    const std::vector<std::byte> empty;
    Reader emptyReader(empty);

    cr_assert_eq(emptyReader.getU8().error(), CodecError::Truncated);
    cr_assert_eq(emptyReader.getU16Le().error(), CodecError::Truncated);
    cr_assert_eq(emptyReader.getU32Le().error(), CodecError::Truncated);
    cr_assert_eq(emptyReader.getU64Le().error(), CodecError::Truncated);
    cr_assert_eq(emptyReader.getBool().error(), CodecError::Truncated);
    cr_assert_eq(emptyReader.getFloatLe().error(), CodecError::Truncated);
    cr_assert_eq(emptyReader.getString().error(), CodecError::Truncated);
    cr_assert_eq(emptyReader.getBytes(1).error(), CodecError::Truncated);
}

Test(network_reader, partial_multi_byte_value_is_truncated)
{
    const std::vector<std::byte> one{std::byte{0x01}};
    Reader reader(one);

    cr_assert_eq(reader.getU16Le().error(), CodecError::Truncated);
}

Test(network_reader, invalid_bool_is_rejected)
{
    Writer writer;

    cr_assert(writer.putU8(2));

    Reader reader(writer.getData());

    cr_assert_eq(reader.getBool().error(), CodecError::InvalidValue);
}

Test(network_serializer, oversized_string_is_refused_without_growth)
{
    Writer writer;
    const std::string tooLong(younity::network::MAX_STRING_BYTES + 1, 'x');

    cr_assert_not(writer.putString(tooLong));
    cr_assert_eq(writer.getSize(), std::size_t{0});
}

Test(network_reader, malicious_string_length_is_rejected_before_alloc)
{
    Writer writer;

    cr_assert(writer.putU16Le(60000));

    Reader reader(writer.getData());

    cr_assert_eq(reader.getString().error(), CodecError::TooLarge);
}

Test(network_reader, short_string_payload_is_truncated)
{
    Writer writer;

    cr_assert(writer.putU16Le(7));
    cr_assert(writer.putBytes(std::vector<std::byte>{std::byte{'h'}, std::byte{'i'}}));

    Reader reader(writer.getData());

    cr_assert_eq(reader.getString().error(), CodecError::Truncated);
}

Test(network_writer, packet_limit_is_enforced)
{
    Writer writer;

    for (std::size_t count = 0; count < younity::network::MAX_PACKET_BYTES; count += 1) {
        cr_assert(writer.putU8(0xFF));
    }
    cr_assert_eq(writer.getSize(), younity::network::MAX_PACKET_BYTES);
    cr_assert_not(writer.putU8(0xFF));
    cr_assert_eq(writer.getSize(), younity::network::MAX_PACKET_BYTES);

    writer.clear();
    cr_assert_eq(writer.getSize(), std::size_t{0});
    cr_assert(writer.putU8(0x01));
}

Test(network_reader, short_bytes_payload_is_truncated)
{
    Writer writer;

    cr_assert(writer.putU8(0xAA));
    cr_assert(writer.putU8(0xBB));

    Reader reader(writer.getData());

    cr_assert_eq(reader.getBytes(3).error(), CodecError::Truncated);
    cr_assert_eq(reader.getRemaining(), std::size_t{2});
}

Test(network_writer, every_width_refuses_past_the_limit)
{
    Writer writer;

    for (std::size_t count = 0; count + 1 < younity::network::MAX_PACKET_BYTES; count += 1) {
        cr_assert(writer.putU8(0xFF));
    }
    cr_assert_eq(writer.getSize(), younity::network::MAX_PACKET_BYTES - 1);

    cr_assert_not(writer.putU16Le(0x1234));
    cr_assert_not(writer.putU32Le(0x12345678));
    cr_assert_not(writer.putU64Le(0x1122334455667788ULL));
    cr_assert_not(writer.putFloatLe(1.5F));
    cr_assert_not(writer.putString("toolong"));
    cr_assert_not(writer.putBytes(std::vector<std::byte>{std::byte{0x01}, std::byte{0x02}}));
    cr_assert(writer.putU8(0xFF));
    cr_assert_eq(writer.getSize(), younity::network::MAX_PACKET_BYTES);
}

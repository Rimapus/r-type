/*
** EPITECH PROJECT, 2026
** younity-engine/fuzz/packet
** File description:
** Packet parser fuzzing harness
*/

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <span>
#include <vector>

#include "younity/network/Packet.hpp"
#include "younity/network/PacketError.hpp"
#include "younity/network/Writer.hpp"

// Feeds arbitrary bytes to the packet parser, run by libFuzzer:
//
//   cmake --preset fuzz
//   cmake --build --preset fuzz
//   ./build/fuzz/fuzz_packet build/fuzz/work fuzz/corpus -max_len=1400 -max_total_time=120
//
// The first corpus directory receives the new inputs libFuzzer finds, so it
// points at the git-ignored build folder while fuzz/corpus holds the seeds.
// Any return of decodePacket is fine, including every PacketError: the bugs
// hunted here are crashes, hangs, leaks and sanitizer reports, plus a broken
// roundtrip (a decoded packet must always re-encode to itself).
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size)
{
    const std::span<const std::byte> bytes{reinterpret_cast<const std::byte *>(data), size};
    const std::expected<younity::network::Packet, younity::network::PacketError> decoded =
        younity::network::decodePacket(bytes);

    if (!decoded.has_value()) {
        return 0;
    }
    younity::network::Writer out;

    if (!younity::network::encodePacket(*decoded, out)) {
        std::abort();
    }
    const std::expected<younity::network::Packet, younity::network::PacketError> recoded =
        younity::network::decodePacket(out.getData());

    if (!recoded.has_value() || recoded->type != decoded->type || recoded->seq != decoded->seq ||
        recoded->payload != decoded->payload) {
        std::abort();
    }
    return 0;
}

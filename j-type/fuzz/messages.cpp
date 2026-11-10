/*
** EPITECH PROJECT, 2026
** j-type/fuzz/messages
** File description:
** Game message codec fuzzing harness
*/

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <span>
#include <variant>
#include <vector>

#include "jtype/common/Messages.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Packet.hpp"

// Feeds arbitrary bytes to the game message decoder, run by libFuzzer:
//
//   cmake --preset fuzz
//   cmake --build --preset fuzz
//   ./build/fuzz/fuzz_messages build/fuzz/work fuzz/corpus -max_len=1200 -max_total_time=120
//
// The first corpus directory receives the new inputs libFuzzer finds, so it
// points at the git-ignored build folder while fuzz/corpus holds the seeds.
// Any return of decodeMessage is fine, including every DecodeError: the bugs
// hunted here are crashes, hangs, leaks and sanitizer reports, plus a broken
// roundtrip. Whatever the decoder accepted must encode back to a datagram that
// decodes to the same message, and every chunk of a split snapshot must do the
// same.
namespace {

/**
 * @brief Decode a datagram and check that every accepted message survives a roundtrip.
 *
 * @param datagram The arbitrary bytes fed by libFuzzer
 */
void checkRoundtrip(std::span<const std::byte> datagram)
{
    const std::expected<younity::network::Packet, younity::network::PacketError> packet =
        younity::network::decodePacket(datagram);

    if (!packet) {
        return;
    }
    const std::expected<jtype::Message, jtype::DecodeError> message = jtype::decodeMessage(*packet);

    if (!message) {
        return;
    }
    const std::vector<std::byte> encoded = jtype::encodeMessage(*message, packet->seq);
    const std::expected<younity::network::Packet, younity::network::PacketError> repacket =
        younity::network::decodePacket(encoded);

    if (encoded.empty() || !repacket || repacket->type != packet->type || repacket->seq != packet->seq) {
        std::abort();
    }
    const std::expected<jtype::Message, jtype::DecodeError> recoded = jtype::decodeMessage(*repacket);

    if (!recoded || *recoded != *message) {
        std::abort();
    }
    const auto *snapshot = std::get_if<jtype::SnapshotMessage>(&*message);

    if (!snapshot) {
        return;
    }
    const std::vector<jtype::SnapshotMessage> chunks = jtype::splitSnapshot(*snapshot);

    if (chunks.empty() || chunks.size() != snapshot->chunkCount) {
        std::abort();
    }
    std::size_t carried = 0;

    for (const jtype::SnapshotMessage &chunk : chunks) {
        const std::vector<std::byte> bytes = jtype::encodeMessage(chunk, packet->seq);
        const std::expected<younity::network::Packet, younity::network::PacketError> chunkPacket =
            younity::network::decodePacket(bytes);

        if (bytes.empty() || !chunkPacket || bytes.size() > younity::network::MAX_PACKET_BYTES) {
            std::abort();
        }
        const std::expected<jtype::Message, jtype::DecodeError> decodedChunk = jtype::decodeMessage(*chunkPacket);

        if (!decodedChunk || *decodedChunk != jtype::Message{chunk}) {
            std::abort();
        }
        carried += chunk.entities.size();
    }
    if (carried != snapshot->entities.size()) {
        std::abort();
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size)
{
    checkRoundtrip({reinterpret_cast<const std::byte *>(data), size});
    return 0;
}

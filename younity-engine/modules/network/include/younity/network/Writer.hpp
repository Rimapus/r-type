/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/Writer
** File description:
** Writer header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace younity::network {

/**
 * @brief Appends little-endian binary fields to a datagram payload.
 *
 * All integers and floats are stored little-endian on the wire so Linux and
 * Windows clients exchange the same bytes. Every put fails with false instead
 * of growing past the packet limit, so callers can drop oversized packets.
 */
class Writer
{
public:
    /**
     * @brief Create an empty writer.
     */
    Writer(void) = default;

    /**
     * @brief Append an 8-bit unsigned integer.
     *
     * @param value The value to append
     *
     * @return false if the packet limit would be exceeded.
     */
    [[nodiscard]] bool putU8(std::uint8_t value) noexcept;

    /**
     * @brief Append a 16-bit unsigned integer in little-endian order.
     *
     * @param value The value to append
     *
     * @return false if the packet limit would be exceeded.
     */
    [[nodiscard]] bool putU16Le(std::uint16_t value) noexcept;

    /**
     * @brief Append a 32-bit unsigned integer in little-endian order.
     *
     * @param value The value to append
     *
     * @return false if the packet limit would be exceeded.
     */
    [[nodiscard]] bool putU32Le(std::uint32_t value) noexcept;

    /**
     * @brief Append a 64-bit unsigned integer in little-endian order.
     *
     * @param value The value to append
     *
     * @return false if the packet limit would be exceeded.
     */
    [[nodiscard]] bool putU64Le(std::uint64_t value) noexcept;

    /**
     * @brief Append a boolean as a single 0 or 1 byte.
     *
     * @param value The value to append
     *
     * @return false if the packet limit would be exceeded.
     */
    [[nodiscard]] bool putBool(bool value) noexcept;

    /**
     * @brief Append a 32-bit float in little-endian order.
     *
     * @param value The value to append
     *
     * @return false if the packet limit would be exceeded.
     */
    [[nodiscard]] bool putFloatLe(float value) noexcept;

    /**
     * @brief Append a string as a 16-bit little-endian length followed by its bytes.
     *
     * @param value The string to append
     *
     * @return false if the string or the packet limit would be exceeded.
     */
    [[nodiscard]] bool putString(std::string_view value) noexcept;

    /**
     * @brief Append raw bytes without any length prefix.
     *
     * @param data The bytes to append
     *
     * @return false if the packet limit would be exceeded.
     */
    [[nodiscard]] bool putBytes(std::span<const std::byte> data) noexcept;

    /**
     * @brief Get the written payload.
     *
     * @return The bytes written so far.
     */
    [[nodiscard]] std::span<const std::byte> getData(void) const noexcept;

    /**
     * @brief Get the size of the written payload.
     *
     * @return The number of bytes written so far.
     */
    [[nodiscard]] std::size_t getSize(void) const noexcept;

    /**
     * @brief Discard the written payload.
     */
    void clear(void) noexcept;

private:
    std::vector<std::byte> _buffer;
};

} // namespace younity::network

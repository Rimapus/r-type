/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/Reader
** File description:
** Reader header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

#include "younity/network/CodecError.hpp"

namespace younity::network {

/**
 * @brief Reads little-endian binary fields from a datagram payload.
 *
 * Every getter bounds-checks before touching the buffer and reports a
 * CodecError instead of throwing, so malformed packets from the network can
 * never crash the caller nor force huge allocations.
 */
class Reader
{
public:
    /**
     * @brief Create a reader over received bytes.
     *
     * @param data The received payload, must outlive the reader
     */
    explicit Reader(std::span<const std::byte> data) noexcept;

    /**
     * @brief Read an 8-bit unsigned integer.
     *
     * @return The value, or CodecError::Truncated if no byte remains.
     */
    [[nodiscard]] std::expected<std::uint8_t, CodecError> getU8(void) noexcept;

    /**
     * @brief Read a 16-bit little-endian unsigned integer.
     *
     * @return The value, or CodecError::Truncated if fewer than 2 bytes remain.
     */
    [[nodiscard]] std::expected<std::uint16_t, CodecError> getU16Le(void) noexcept;

    /**
     * @brief Read a 32-bit little-endian unsigned integer.
     *
     * @return The value, or CodecError::Truncated if fewer than 4 bytes remain.
     */
    [[nodiscard]] std::expected<std::uint32_t, CodecError> getU32Le(void) noexcept;

    /**
     * @brief Read a 64-bit little-endian unsigned integer.
     *
     * @return The value, or CodecError::Truncated if fewer than 8 bytes remain.
     */
    [[nodiscard]] std::expected<std::uint64_t, CodecError> getU64Le(void) noexcept;

    /**
     * @brief Read a boolean encoded as a single 0 or 1 byte.
     *
     * @return The value, CodecError::Truncated if no byte remains,
     *         or CodecError::InvalidValue for any other byte.
     */
    [[nodiscard]] std::expected<bool, CodecError> getBool(void) noexcept;

    /**
     * @brief Read a 32-bit little-endian float.
     *
     * @return The value, or CodecError::Truncated if fewer than 4 bytes remain.
     */
    [[nodiscard]] std::expected<float, CodecError> getFloatLe(void) noexcept;

    /**
     * @brief Read a string encoded as a 16-bit length followed by its bytes.
     *
     * @return The value, CodecError::Truncated if the announced bytes are missing,
     *         or CodecError::TooLarge if the announced length exceeds the string limit.
     */
    [[nodiscard]] std::expected<std::string, CodecError> getString(void);

    /**
     * @brief Read raw bytes without any length prefix.
     *
     * @param size The number of bytes to read
     *
     * @return The bytes, or CodecError::Truncated if fewer bytes remain.
     */
    [[nodiscard]] std::expected<std::vector<std::byte>, CodecError> getBytes(std::size_t size);

    /**
     * @brief Get the number of unread bytes.
     *
     * @return The number of bytes not read yet.
     */
    [[nodiscard]] std::size_t getRemaining(void) const noexcept;

private:
    std::span<const std::byte> _data;
    std::size_t _offset = 0;
};

} // namespace younity::network

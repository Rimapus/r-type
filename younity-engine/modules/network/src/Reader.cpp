/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/Reader
** File description:
** Reader source file
*/

#include "younity/network/Reader.hpp"

#include <bit>

#include "younity/network/Limits.hpp"

namespace younity::network {

Reader::Reader(std::span<const std::byte> data) noexcept
    : _data(data)
{
}

std::expected<std::uint8_t, CodecError> Reader::getU8(void) noexcept
{
    if (this->getRemaining() < sizeof(std::uint8_t)) {
        return std::unexpected(CodecError::Truncated);
    }
    const std::uint8_t value = static_cast<std::uint8_t>(this->_data[this->_offset]);

    this->_offset += sizeof(std::uint8_t);
    return value;
}

std::expected<std::uint16_t, CodecError> Reader::getU16Le(void) noexcept
{
    if (this->getRemaining() < sizeof(std::uint16_t)) {
        return std::unexpected(CodecError::Truncated);
    }
    const std::uint16_t low = static_cast<std::uint8_t>(this->_data[this->_offset]);
    const std::uint16_t high = static_cast<std::uint8_t>(this->_data[this->_offset + 1]);

    this->_offset += sizeof(std::uint16_t);
    return static_cast<std::uint16_t>(low | (high << 8U));
}

std::expected<std::uint32_t, CodecError> Reader::getU32Le(void) noexcept
{
    if (this->getRemaining() < sizeof(std::uint32_t)) {
        return std::unexpected(CodecError::Truncated);
    }
    std::uint32_t value = 0;

    for (std::size_t index = 0; index < sizeof(std::uint32_t); index += 1) {
        const std::uint32_t byte = static_cast<std::uint8_t>(this->_data[this->_offset + index]);

        value |= byte << (index * 8U);
    }
    this->_offset += sizeof(std::uint32_t);
    return value;
}

std::expected<std::uint64_t, CodecError> Reader::getU64Le(void) noexcept
{
    if (this->getRemaining() < sizeof(std::uint64_t)) {
        return std::unexpected(CodecError::Truncated);
    }
    std::uint64_t value = 0;

    for (std::size_t index = 0; index < sizeof(std::uint64_t); index += 1) {
        const std::uint64_t byte = static_cast<std::uint8_t>(this->_data[this->_offset + index]);

        value |= byte << (index * 8U);
    }
    this->_offset += sizeof(std::uint64_t);
    return value;
}

std::expected<bool, CodecError> Reader::getBool(void) noexcept
{
    const std::expected<std::uint8_t, CodecError> byte = this->getU8();

    if (!byte.has_value()) {
        return std::unexpected(byte.error());
    }
    if (*byte == 0) {
        return false;
    }
    if (*byte == 1) {
        return true;
    }
    return std::unexpected(CodecError::InvalidValue);
}

std::expected<float, CodecError> Reader::getFloatLe(void) noexcept
{
    const std::expected<std::uint32_t, CodecError> bits = this->getU32Le();

    if (!bits.has_value()) {
        return std::unexpected(bits.error());
    }
    return std::bit_cast<float>(*bits);
}

std::expected<std::string, CodecError> Reader::getString(void)
{
    const std::expected<std::uint16_t, CodecError> length = this->getU16Le();

    if (!length.has_value()) {
        return std::unexpected(length.error());
    }
    if (*length > MAX_STRING_BYTES) {
        return std::unexpected(CodecError::TooLarge);
    }
    if (this->getRemaining() < *length) {
        return std::unexpected(CodecError::Truncated);
    }
    std::string value;

    value.reserve(*length);
    for (std::uint16_t index = 0; index < *length; index += 1) {
        value.push_back(static_cast<char>(this->_data[this->_offset + index]));
    }
    this->_offset += *length;
    return value;
}

std::expected<std::vector<std::byte>, CodecError> Reader::getBytes(std::size_t size)
{
    if (this->getRemaining() < size) {
        return std::unexpected(CodecError::Truncated);
    }
    std::vector<std::byte> value(this->_data.begin() + this->_offset, this->_data.begin() + this->_offset + size);

    this->_offset += size;
    return value;
}

std::size_t Reader::getRemaining(void) const noexcept
{
    return this->_data.size() - this->_offset;
}

} // namespace younity::network

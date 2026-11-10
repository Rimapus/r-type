/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/Writer
** File description:
** Writer source file
*/

#include "younity/network/Writer.hpp"

#include <bit>

#include "younity/network/Limits.hpp"

namespace younity::network {

bool Writer::putU8(std::uint8_t value) noexcept
{
    if (this->_buffer.size() + sizeof(value) > MAX_PACKET_BYTES) {
        return false;
    }
    this->_buffer.push_back(static_cast<std::byte>(value));
    return true;
}

bool Writer::putU16Le(std::uint16_t value) noexcept
{
    if (this->_buffer.size() + sizeof(value) > MAX_PACKET_BYTES) {
        return false;
    }
    this->_buffer.push_back(static_cast<std::byte>(value & 0xFFU));
    this->_buffer.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
    return true;
}

bool Writer::putU32Le(std::uint32_t value) noexcept
{
    if (this->_buffer.size() + sizeof(value) > MAX_PACKET_BYTES) {
        return false;
    }
    this->_buffer.push_back(static_cast<std::byte>(value & 0xFFU));
    this->_buffer.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
    this->_buffer.push_back(static_cast<std::byte>((value >> 16U) & 0xFFU));
    this->_buffer.push_back(static_cast<std::byte>((value >> 24U) & 0xFFU));
    return true;
}

bool Writer::putU64Le(std::uint64_t value) noexcept
{
    if (this->_buffer.size() + sizeof(value) > MAX_PACKET_BYTES) {
        return false;
    }
    for (unsigned int shift = 0; shift < 64; shift += 8) {
        this->_buffer.push_back(static_cast<std::byte>((value >> shift) & 0xFFULL));
    }
    return true;
}

bool Writer::putBool(bool value) noexcept
{
    return this->putU8(value ? std::uint8_t{1} : std::uint8_t{0});
}

bool Writer::putFloatLe(float value) noexcept
{
    return this->putU32Le(std::bit_cast<std::uint32_t>(value));
}

bool Writer::putString(std::string_view value) noexcept
{
    if (value.size() > MAX_STRING_BYTES) {
        return false;
    }
    if (this->_buffer.size() + sizeof(std::uint16_t) + value.size() > MAX_PACKET_BYTES) {
        return false;
    }
    if (!this->putU16Le(static_cast<std::uint16_t>(value.size()))) {
        return false;
    }
    for (const char letter : value) {
        this->_buffer.push_back(static_cast<std::byte>(letter));
    }
    return true;
}

bool Writer::putBytes(std::span<const std::byte> data) noexcept
{
    if (this->_buffer.size() + data.size() > MAX_PACKET_BYTES) {
        return false;
    }
    this->_buffer.insert(this->_buffer.end(), data.begin(), data.end());
    return true;
}

std::span<const std::byte> Writer::getData(void) const noexcept
{
    return std::span<const std::byte>(this->_buffer.data(), this->_buffer.size());
}

std::size_t Writer::getSize(void) const noexcept
{
    return this->_buffer.size();
}

void Writer::clear(void) noexcept
{
    this->_buffer.clear();
}

} // namespace younity::network

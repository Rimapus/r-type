/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/Endpoint
** File description:
** Endpoint source file
*/

#include "younity/network/Endpoint.hpp"

#include <string>
#include <utility>

#include <asio.hpp>

namespace younity::network {

Endpoint Endpoint::loopback(std::uint16_t port)
{
    return Endpoint("127.0.0.1", port);
}

std::expected<Endpoint, SocketError> Endpoint::resolve(std::string_view host, std::uint16_t port)
{
    // The system would give the local address for an empty name: a host left empty is a mistake to report
    if (host.empty()) {
        return std::unexpected(SocketError::InvalidAddress);
    }
    asio::io_context context;
    asio::ip::udp::resolver resolver(context);
    asio::error_code code;
    // IPv4 only: the sockets of the engine are IPv4 sockets
    const asio::ip::udp::resolver::results_type results =
        resolver.resolve(asio::ip::udp::v4(), std::string(host), std::to_string(port), code);

    if (code || results.empty()) {
        return std::unexpected(SocketError::InvalidAddress);
    }
    return Endpoint(results.begin()->endpoint().address().to_string(), port);
}

Endpoint::Endpoint(std::string address, std::uint16_t port)
    : _address(std::move(address)),
      _port(port)
{
}

const std::string &Endpoint::getAddress(void) const noexcept
{
    return this->_address;
}

std::uint16_t Endpoint::getPort(void) const noexcept
{
    return this->_port;
}

std::string Endpoint::toString(void) const
{
    return this->_address + ":" + std::to_string(this->_port);
}

} // namespace younity::network

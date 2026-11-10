/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/Endpoint
** File description:
** Endpoint header file
*/

#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

#include "younity/network/SocketError.hpp"

namespace younity::network {

/**
 * @brief An IPv4 address and port identifying a UDP peer.
 *
 * Plain value type without any Asio dependency: the socket converts it to and
 * from its own endpoint representation internally.
 */
class Endpoint
{
public:
    /**
     * @brief Create the loopback endpoint for the given port.
     *
     * @param port The port, e.g. the one reported by a bound socket
     *
     * @return The 127.0.0.1 endpoint for tests and local games.
     */
    [[nodiscard]] static Endpoint loopback(std::uint16_t port);

    /**
     * @brief Find the endpoint of a host, given by its name or by its address written as text,
     *        e.g. a client joining the server typed on its command line. A name is looked up by the system,
     *        which can wait on the network.
     *
     * @param host The host, e.g. "localhost", "game.example.org" or "192.168.1.10"
     * @param port The port
     *
     * @return The endpoint of the first IPv4 address of the host, or SocketError::InvalidAddress if the host is
     *         empty or has no IPv4 address.
     */
    [[nodiscard]] static std::expected<Endpoint, SocketError> resolve(std::string_view host, std::uint16_t port);

    /**
     * @brief Create an endpoint from an address and a port.
     *
     * @param address The dotted IPv4 address, e.g. "127.0.0.1"
     * @param port The port
     */
    Endpoint(std::string address, std::uint16_t port);

    /**
     * @brief Get the dotted IPv4 address.
     *
     * @return The address, e.g. "127.0.0.1".
     */
    [[nodiscard]] const std::string &getAddress(void) const noexcept;

    /**
     * @brief Get the port.
     *
     * @return The port.
     */
    [[nodiscard]] std::uint16_t getPort(void) const noexcept;

    /**
     * @brief Get the endpoint as text.
     *
     * @return The address followed by the port, e.g. "127.0.0.1:40001".
     */
    [[nodiscard]] std::string toString(void) const;

    /**
     * @brief Check if two endpoints hold the same address and port.
     */
    bool operator==(const Endpoint &other) const noexcept = default;

private:
    std::string _address = "0.0.0.0";
    std::uint16_t _port = 0;
};

} // namespace younity::network

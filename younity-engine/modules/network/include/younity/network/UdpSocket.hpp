/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/UdpSocket
** File description:
** UdpSocket header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>

#include "younity/network/Endpoint.hpp"
#include "younity/network/SocketError.hpp"

namespace younity::network {

/**
 * @brief Blocking IPv4 UDP socket, backed by Asio.
 *
 * Minimal sync building block for the engine: one socket serves all peers and
 * demultiplexes them via Endpoint. UdpPump adds the background threads for the
 * game loop; game code only ever sees byte spans and expected errors, never Asio.
 */
class UdpSocket
{
public:
    /**
     * @brief Create a closed socket.
     */
    UdpSocket(void);

    /**
     * @brief Close the socket.
     */
    ~UdpSocket(void);

    UdpSocket(const UdpSocket &) = delete;
    UdpSocket &operator=(const UdpSocket &) = delete;

    /**
     * @brief Move a socket, leaving the source closed.
     */
    UdpSocket(UdpSocket &&other) noexcept;

    /**
     * @brief Move a socket, leaving the source closed.
     */
    UdpSocket &operator=(UdpSocket &&other) noexcept;

    /**
     * @brief Check if the socket is open.
     *
     * @return true once bound or implicitly opened by a send.
     */
    [[nodiscard]] bool isOpen(void) const noexcept;

    /**
     * @brief Bind the socket to all interfaces on the given port.
     *
     * @param port The port to listen on, or 0 for an ephemeral port
     *
     * @return The error, or nothing on success. Use localEndpoint to learn
     *         the ephemeral port.
     */
    [[nodiscard]] std::expected<void, SocketError> bind(std::uint16_t port);

    /**
     * @brief Bind the socket to a specific address and port.
     *
     * @param endpoint The address and port to listen on, e.g.
     *        Endpoint::loopback(0) for localhost with an ephemeral port
     *
     * @return The error, or nothing on success. InvalidAddress if the address
     *         is not a valid IPv4 address.
     */
    [[nodiscard]] std::expected<void, SocketError> bind(const Endpoint &endpoint);

    /**
     * @brief Send bytes to a peer, opening the socket if needed.
     *
     * @param data The bytes to send
     * @param remote The peer to send to
     *
     * @return The bytes sent, or the socket error.
     */
    [[nodiscard]] std::expected<std::size_t, SocketError> sendTo(
        std::span<const std::byte> data,
        const Endpoint &remote);

    /**
     * @brief Receive bytes from any peer.
     *
     * @param buffer Where to store the datagram, sized by the caller
     * @param sender Set to the peer the datagram came from on success
     *
     * @return The bytes received, or the socket error. Blocks until a
     *         datagram arrives, so callers must know one is coming. A datagram
     *         larger than the buffer is cut to its size on Linux and gives
     *         SocketError::TooLarge on Windows.
     */
    [[nodiscard]] std::expected<std::size_t, SocketError> receiveFrom(
        std::span<std::byte> buffer,
        Endpoint &sender);

    /**
     * @brief Get the local address and port.
     *
     * @return The bound endpoint, or the socket error if not open.
     */
    [[nodiscard]] std::expected<Endpoint, SocketError> getLocalEndpoint(void) const;

    /**
     * @brief Close the socket.
     */
    void close(void) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace younity::network

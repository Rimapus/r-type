/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/UdpSocket
** File description:
** UdpSocket source file
*/

#include "younity/network/UdpSocket.hpp"

#include <asio.hpp>

namespace younity::network {

struct UdpSocket::Impl
{
    asio::io_context context;
    asio::ip::udp::socket socket;

    Impl(void)
        : socket(context)
    {
    }
};

namespace {

[[nodiscard]] std::expected<asio::ip::udp::endpoint, SocketError> toAsioEndpoint(const Endpoint &endpoint)
{
    asio::error_code code;
    const asio::ip::address address = asio::ip::make_address(endpoint.getAddress(), code);

    if (code) {
        return std::unexpected(SocketError::InvalidAddress);
    }
    return asio::ip::udp::endpoint(address, endpoint.getPort());
}

[[nodiscard]] Endpoint fromAsioEndpoint(const asio::ip::udp::endpoint &endpoint)
{
    return Endpoint(endpoint.address().to_string(), endpoint.port());
}

} // namespace

UdpSocket::UdpSocket(void)
    : _impl(std::make_unique<Impl>())
{
}

UdpSocket::~UdpSocket(void) = default;

UdpSocket::UdpSocket(UdpSocket &&other) noexcept = default;

UdpSocket &UdpSocket::operator=(UdpSocket &&other) noexcept = default;

bool UdpSocket::isOpen(void) const noexcept
{
    return this->_impl != nullptr && this->_impl->socket.is_open();
}

std::expected<void, SocketError> UdpSocket::bind(std::uint16_t port)
{
    return this->bind(Endpoint("0.0.0.0", port));
}

std::expected<void, SocketError> UdpSocket::bind(const Endpoint &endpoint)
{
    const std::expected<asio::ip::udp::endpoint, SocketError> local = toAsioEndpoint(endpoint);

    if (!local.has_value()) {
        return std::unexpected(local.error());
    }
    asio::error_code code;

    this->_impl->socket.open(asio::ip::udp::v4(), code);
    if (code) {
        return std::unexpected(SocketError::OpenFailed);
    }
    this->_impl->socket.bind(*local, code);
    if (code) {
        return std::unexpected(SocketError::BindFailed);
    }
    return {};
}

std::expected<std::size_t, SocketError> UdpSocket::sendTo(std::span<const std::byte> data, const Endpoint &remote)
{
    const std::expected<asio::ip::udp::endpoint, SocketError> destination = toAsioEndpoint(remote);

    if (!destination.has_value()) {
        return std::unexpected(destination.error());
    }
    if (!this->isOpen()) {
        asio::error_code code;

        this->_impl->socket.open(asio::ip::udp::v4(), code);
        if (code) {
            return std::unexpected(SocketError::OpenFailed);
        }
    }
    asio::error_code code;
    const std::size_t sent = this->_impl->socket.send_to(asio::buffer(data.data(), data.size()), *destination, 0, code);

    if (code) {
        return std::unexpected(SocketError::SendFailed);
    }
    return sent;
}

std::expected<std::size_t, SocketError> UdpSocket::receiveFrom(std::span<std::byte> buffer, Endpoint &sender)
{
    if (!this->isOpen()) {
        return std::unexpected(SocketError::NotOpen);
    }
    asio::ip::udp::endpoint from;
    asio::error_code code;
    const std::size_t received =
        this->_impl->socket.receive_from(asio::buffer(buffer.data(), buffer.size()), from, 0, code);

    if (code == asio::error::message_size) {
        return std::unexpected(SocketError::TooLarge);
    }
    if (code) {
        return std::unexpected(SocketError::ReceiveFailed);
    }
    sender = fromAsioEndpoint(from);
    return received;
}

std::expected<Endpoint, SocketError> UdpSocket::getLocalEndpoint(void) const
{
    if (!this->isOpen()) {
        return std::unexpected(SocketError::NotOpen);
    }
    asio::error_code code;
    const asio::ip::udp::endpoint local = this->_impl->socket.local_endpoint(code);

    if (code) {
        return std::unexpected(SocketError::ReceiveFailed);
    }
    return fromAsioEndpoint(local);
}

void UdpSocket::close(void) noexcept
{
    if (this->_impl != nullptr) {
        asio::error_code code;

        this->_impl->socket.close(code);
    }
}

} // namespace younity::network

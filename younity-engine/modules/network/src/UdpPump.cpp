/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/UdpPump
** File description:
** UdpPump source file
*/

#include "younity/network/UdpPump.hpp"

#include <chrono>
#include <system_error>
#include <vector>

#include "younity/network/Limits.hpp"
#include "younity/network/Log.hpp"

namespace younity::network {

namespace {

// The shortest delay between two warnings of a failing receive: a peer sending garbage must not flood the log
constexpr std::chrono::seconds RECEIVE_WARNING_INTERVAL{1};

} // namespace

UdpPump::UdpPump(std::size_t queueCapacity)
    : _incoming(queueCapacity), _outgoing(queueCapacity)
{
}

UdpPump::~UdpPump(void)
{
    this->stop();
}

bool UdpPump::isOpen(void) const noexcept
{
    return this->_socket.isOpen();
}

bool UdpPump::isRunning(void) const noexcept
{
    return this->_running.load();
}

std::expected<void, SocketError> UdpPump::bind(std::uint16_t port)
{
    if (this->_running.load()) {
        return std::unexpected(SocketError::BindFailed);
    }
    return this->_socket.bind(port);
}

std::expected<void, SocketError> UdpPump::bind(const Endpoint &endpoint)
{
    if (this->_running.load()) {
        return std::unexpected(SocketError::BindFailed);
    }
    return this->_socket.bind(endpoint);
}

std::expected<void, SocketError> UdpPump::start(void)
{
    if (this->_running.load()) {
        return {};
    }
    if (!this->_socket.isOpen()) {
        return std::unexpected(SocketError::NotOpen);
    }
    this->_incoming.reset();
    this->_outgoing.reset();
    this->_running.store(true);
    try {
        // The send thread goes first: if the receive thread cannot start, closing
        // the outgoing queue wakes the send thread and the socket can then close.
        this->_sendThread = std::thread(&UdpPump::runSend, this);
        this->_recvThread = std::thread(&UdpPump::runReceive, this);
    } catch (const std::system_error &) {
        this->_running.store(false);
        this->_outgoing.close();
        if (this->_sendThread.joinable()) {
            this->_sendThread.join();
        }
        this->_socket.close();
        return std::unexpected(SocketError::OpenFailed);
    }
    return {};
}

void UdpPump::stop(void) noexcept
{
    this->_running.store(false);
    this->_outgoing.close();
    if (this->_sendThread.joinable()) {
        this->_sendThread.join();
    }
    if (this->_recvThread.joinable()) {
        // Closing a socket does not wake a thread blocked in a synchronous
        // receive on Linux: send it one byte so it wakes, sees the flag and
        // exits. A socket bound to any address is reached on the loopback, as
        // a datagram sent to 0.0.0.0 fails on Windows.
        const std::expected<Endpoint, SocketError> local = this->_socket.getLocalEndpoint();
        bool woken = false;

        if (local.has_value()) {
            const Endpoint self = local->getAddress() == "0.0.0.0" ? Endpoint::loopback(local->getPort()) : *local;
            const std::byte wake{0};

            woken = this->_socket.sendTo(std::span<const std::byte>(&wake, 1), self).has_value();
        }
        // Without the byte, closing the socket is the last way out of the receive
        if (!woken) {
            this->_socket.close();
        }
        this->_recvThread.join();
    }
    this->_socket.close();
}

bool UdpPump::sendTo(const Endpoint &to, std::span<const std::byte> bytes)
{
    if (bytes.size() > MAX_PACKET_BYTES) {
        return false;
    }
    SendRequest request;

    request.to = to;
    request.bytes.assign(bytes.begin(), bytes.end());
    return this->_outgoing.push(std::move(request));
}

std::optional<ReceivedDatagram> UdpPump::tryReceive(void)
{
    return this->_incoming.tryPop();
}

std::optional<ReceivedDatagram> UdpPump::waitReceive(void)
{
    return this->_incoming.waitPop();
}

std::expected<Endpoint, SocketError> UdpPump::getLocalEndpoint(void) const
{
    return this->_socket.getLocalEndpoint();
}

std::size_t UdpPump::getIncomingSize(void) const
{
    return this->_incoming.getSize();
}

std::size_t UdpPump::getOutgoingSize(void) const
{
    return this->_outgoing.getSize();
}

std::size_t UdpPump::getDroppedIncoming(void) const noexcept
{
    return this->_droppedIncoming.load(std::memory_order_relaxed);
}

void UdpPump::runReceive(void)
{
    // One byte more than a packet: a datagram that fills it is too large, whatever the system cut from it
    std::vector<std::byte> buffer(MAX_PACKET_BYTES + 1);
    std::chrono::steady_clock::time_point lastWarning{};
    std::size_t failures = 0;

    while (this->_running.load()) {
        Endpoint sender = Endpoint::loopback(0);
        const std::expected<std::size_t, SocketError> received = this->_socket.receiveFrom(buffer, sender);

        if (!this->_running.load()) {
            break;
        }
        if (!received.has_value() && received.error() != SocketError::TooLarge) {
            const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

            // A short failure must not spin the thread nor kill it: the server
            // keeps running whatever a client sends, and so does the pump.
            ++failures;
            if (now - lastWarning >= RECEIVE_WARNING_INTERVAL) {
                logger().warn("Network receive failed: {} ({} since the last warning)", toString(received.error()), failures);
                lastWarning = now;
                failures = 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        if (!received.has_value() || *received > MAX_PACKET_BYTES) {
            // Never queued: the game would read a datagram the system cut to the size of the buffer
            this->_droppedIncoming.fetch_add(1, std::memory_order_relaxed);
            continue;
        }
        ReceivedDatagram datagram;

        datagram.from = sender;
        datagram.bytes.assign(buffer.begin(), buffer.begin() + *received);
        if (!this->_incoming.push(std::move(datagram)) && this->_running.load()) {
            this->_droppedIncoming.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

void UdpPump::runSend(void)
{
    while (this->_running.load()) {
        std::optional<SendRequest> request = this->_outgoing.waitPop();

        if (!request.has_value()) {
            break;
        }
        if (!this->_running.load()) {
            break;
        }
        const std::expected<std::size_t, SocketError> sent = this->_socket.sendTo(request->bytes, request->to);

        if (!sent.has_value()) {
            logger().warn("Network send to {} failed: {}", request->to.toString(), toString(sent.error()));
        }
    }
}

} // namespace younity::network

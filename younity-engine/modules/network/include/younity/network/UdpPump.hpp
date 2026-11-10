/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/UdpPump
** File description:
** UdpPump header file
*/

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <thread>

#include "younity/network/Datagram.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/SocketError.hpp"
#include "younity/network/ThreadSafeQueue.hpp"
#include "younity/network/UdpSocket.hpp"

namespace younity::network {

/**
 * @brief UDP socket with two background threads, so the game loop never blocks.
 *
 * One socket serves all peers like UdpSocket, but here a receive thread pushes
 * every datagram to an incoming queue and a send thread sends every queued
 * request: the game only calls sendTo and tryReceive, which never wait for
 * the network. Poll tryReceive once per fixed step and hand the bytes to
 * decodePacket. Only this threaded I/O lets the server run frame after frame
 * regardless of what the clients send.
 *
 * Stopping closes the socket to wake the receive thread and closes the send
 * queue to wake the send thread, then joins both. The incoming queue stays
 * readable after the stop, so the game can drain what arrived. Starting again
 * drops whatever was left, both incoming and unsent.
 */
class UdpPump
{
public:
    /**
     * @brief Create a closed pump, bind it before starting it.
     *
     * @param queueCapacity The largest number of datagrams kept in each queue
     */
    explicit UdpPump(std::size_t queueCapacity = DEFAULT_QUEUE_CAPACITY);

    /**
     * @brief Stop the pump and join the threads.
     */
    ~UdpPump(void);

    UdpPump(const UdpPump &) = delete;
    UdpPump &operator=(const UdpPump &) = delete;
    UdpPump(UdpPump &&other) = delete;
    UdpPump &operator=(UdpPump &&other) = delete;

    /**
     * @brief Check if the socket is open.
     *
     * @return true once bound. Stopping closes the socket.
     */
    [[nodiscard]] bool isOpen(void) const noexcept;

    /**
     * @brief Check if the background threads are running.
     *
     * @return true between a successful start and a stop.
     */
    [[nodiscard]] bool isRunning(void) const noexcept;

    /**
     * @brief Bind the socket to all interfaces on the given port.
     *        Refused while the pump runs: stop it first.
     *
     * @param port The port to listen on, or 0 for an ephemeral port
     *
     * @return BindFailed if the pump runs, or the socket error.
     */
    [[nodiscard]] std::expected<void, SocketError> bind(std::uint16_t port);

    /**
     * @brief Bind the socket to a specific address and port.
     *        Refused while the pump runs: stop it first.
     *
     * @param endpoint The address and port to listen on, e.g.
     *        Endpoint::loopback(0) for localhost with an ephemeral port
     *
     * @return BindFailed if the pump runs, or the socket error.
     */
    [[nodiscard]] std::expected<void, SocketError> bind(const Endpoint &endpoint);

    /**
     * @brief Start the receive and send threads.
     *
     * Drops whatever the queues still held from a previous run.
     *
     * @return NotOpen if the socket is not bound, OpenFailed if a thread
     *         could not be created. Starting twice succeeds doing nothing.
     */
    [[nodiscard]] std::expected<void, SocketError> start(void);

    /**
     * @brief Stop the threads and close the socket, does nothing if stopped.
     *        Wakes the receive thread with one byte to itself, on the loopback
     *        when the socket is bound to any address; the byte is dropped and
     *        never queued. The socket is closed once both threads are done with
     *        it. The incoming queue stays readable, unsent requests are dropped
     *        on the next start.
     */
    void stop(void) noexcept;

    /**
     * @brief Queue bytes for sending, without waiting for the network.
     *
     * @param to The peer to send to
     * @param bytes One whole datagram, built with Writer and encodePacket
     *
     * @return false if the datagram exceeds the packet limit, the outgoing
     *         queue is full, or the pump is stopped and the request was dropped.
     */
    [[nodiscard]] bool sendTo(const Endpoint &to, std::span<const std::byte> bytes);

    /**
     * @brief Take the oldest received datagram without waiting.
     *
     * @return The datagram and its sender, or nothing if none arrived.
     */
    [[nodiscard]] std::optional<ReceivedDatagram> tryReceive(void);

    /**
     * @brief Take the oldest received datagram, waiting until one arrives.
     *        Only wakes on arrival: stopping the pump does not wake it, so
     *        prefer tryReceive in the game loop and keep this for the tests.
     *
     * @return The datagram and its sender.
     */
    [[nodiscard]] std::optional<ReceivedDatagram> waitReceive(void);

    /**
     * @brief Get the local address and port.
     *
     * @return The bound endpoint, or the socket error if not open.
     */
    [[nodiscard]] std::expected<Endpoint, SocketError> getLocalEndpoint(void) const;

    /**
     * @brief Get the number of received datagrams waiting to be read.
     *
     * @return The number of values in the incoming queue.
     */
    [[nodiscard]] std::size_t getIncomingSize(void) const;

    /**
     * @brief Get the number of send requests waiting to be sent.
     *
     * @return The number of values in the outgoing queue.
     */
    [[nodiscard]] std::size_t getOutgoingSize(void) const;

    /**
     * @brief Get the number of received datagrams the pump dropped: larger than MAX_PACKET_BYTES, or received
     *        while its queue was full.
     *
     * @return The count since the pump was created.
     */
    [[nodiscard]] std::size_t getDroppedIncoming(void) const noexcept;

private:
    /**
     * @brief Push every received datagram to the incoming queue until stopped.
     *        Send and receive run on distinct threads sharing one UDP socket,
     *        which the operating system handles concurrently.
     */
    void runReceive(void);

    /**
     * @brief Send every queued request until stopped.
     *        Failures are logged and dropped, never requeued.
     */
    void runSend(void);

    UdpSocket _socket;
    ThreadSafeQueue<ReceivedDatagram> _incoming;
    ThreadSafeQueue<SendRequest> _outgoing;
    std::thread _recvThread;
    std::thread _sendThread;
    std::atomic<bool> _running{false};
    std::atomic<std::size_t> _droppedIncoming{0};
};

} // namespace younity::network

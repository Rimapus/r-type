/*
** EPITECH PROJECT, 2026
** j-type/client/src/network/Connection
** File description:
** Connection source file
*/

#include "network/Connection.hpp"

#include <chrono>
#include <expected>
#include <stdexcept>
#include <thread>
#include <utility>
#include <variant>

#include "jtype/common/Log.hpp"
#include "younity/network/Packet.hpp"

namespace jtype {

namespace {

// The delay between two Join while the server has not answered, in seconds
constexpr float JOIN_DELAY = 0.5F;

// The longest wait for the Leave to be sent when the window closes
constexpr std::chrono::milliseconds LEAVE_TIMEOUT{100};

} // namespace

Connection::Connection(younity::network::Endpoint server, younity::network::SessionConfig config)
    : _server(std::move(server)),
      _session(config)
{
    if (!this->_pump.bind(0) || !this->_pump.start()) {
        throw std::runtime_error("cannot open the socket");
    }
}

Received Connection::receive(void)
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    Received received;

    for (auto datagram = this->_pump.tryReceive(); datagram; datagram = this->_pump.tryReceive()) {
        const auto packet = younity::network::decodePacket(datagram->bytes);

        this->_traffic.addReceived(datagram->bytes.size());

        if (datagram->from != this->_server || !packet) {
            continue;
        }
        this->_session.onPacket(datagram->from, *packet, now);

        std::expected<Message, DecodeError> message = decodeMessage(*packet);

        if (!message) {
            continue;
        }
        if (const auto *welcome = std::get_if<WelcomeMessage>(&*message)) {
            if (!this->_slot) {
                logger().info("Joined as the player {}", welcome->slot + 1);
            }
            this->_slot = welcome->slot;
        } else if (std::holds_alternative<FullMessage>(*message)) {
            throw std::runtime_error("the server is full");
        } else if (auto *chunk = std::get_if<SnapshotMessage>(&*message)) {
            if (std::optional<SnapshotMessage> snapshot = this->_assembler.push(std::move(*chunk))) {
                this->_traffic.addSnapshot();
                received.snapshots.push_back(std::move(*snapshot));
            }
        } else if (const auto *scoreboard = std::get_if<ScoreboardMessage>(&*message)) {
            this->_scoreboard = *scoreboard;
        } else if (getEventTick(*message)) {
            received.events.push_back(std::move(*message));
        }
    }
    (void)this->_traffic.update(now);
    // The server answers the heartbeats before the slot and sends a snapshot at each step after: silent, it is gone
    if (this->_session.update(now) == younity::network::SessionState::TimedOut) {
        throw std::runtime_error(this->_slot ? "the connection to the server was lost" : "the server is unreachable");
    }
    return received;
}

void Connection::join(float step)
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    // The session sends its Connect again until the server answers it
    const auto request = this->_session.getState() == younity::network::SessionState::Disconnected
                             ? this->_session.connect(this->_server, now)
                             : this->_session.pollHeartbeat(now);

    if (request) {
        this->send(request->to, request->bytes);
    }
    // The server ignores the Join of a client without a session
    if (this->_session.getState() != younity::network::SessionState::Connected) {
        return;
    }
    this->_joinTimer -= step;
    if (this->_joinTimer <= 0.0F) {
        this->_joinTimer = JOIN_DELAY;
        this->send(this->_server, encodeMessage(JoinMessage{}, this->_session.nextSequence()));
    }
}

std::uint32_t Connection::sendInputs(std::uint8_t inputs)
{
    // From 1: a last input of 0 in the snapshots means that the server applied none yet
    const InputMessage message{inputs, ++this->_tick};

    this->send(this->_server, encodeMessage(message, this->_session.nextSequence()));
    return message.tick;
}

void Connection::leave(void)
{
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + LEAVE_TIMEOUT;

    if (!this->_slot) {
        return;
    }
    this->send(this->_server, encodeMessage(LeaveMessage{}, this->_session.nextSequence()));
    // Stopping the pump drops what is still queued: give the send thread the time to take the Leave
    while (this->_pump.getOutgoingSize() > 0 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Connection::send(const younity::network::Endpoint &to, std::span<const std::byte> bytes)
{
    // A datagram the full queue dropped never reaches the network
    if (this->_pump.sendTo(to, bytes)) {
        this->_traffic.addSent(bytes.size());
    }
}

std::optional<std::uint8_t> Connection::getSlot(void) const
{
    return this->_slot;
}

const ScoreboardMessage &Connection::getScoreboard(void) const
{
    return this->_scoreboard;
}

const younity::network::Endpoint &Connection::getServer(void) const
{
    return this->_server;
}

} // namespace jtype

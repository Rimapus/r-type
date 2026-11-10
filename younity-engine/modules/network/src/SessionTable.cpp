/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/src/SessionTable
** File description:
** SessionTable source file
*/

#include "younity/network/SessionTable.hpp"

#include <vector>

#include "SessionPacket.hpp"

namespace younity::network {

SessionTable::SessionTable(std::chrono::milliseconds timeout, std::size_t maxSessions) noexcept
    : _timeout(timeout), _maxSessions(maxSessions)
{
}

std::optional<SendRequest> SessionTable::onPacket(
    const Endpoint &from, const Packet &packet, std::chrono::steady_clock::time_point now)
{
    if (packet.type == PacketType::Connect) {
        const std::string key = from.toString();
        auto found = this->_sessions.find(key);

        if (found == this->_sessions.end()) {
            if (this->_sessions.size() >= this->_maxSessions) {
                return std::nullopt;
            }
            found = this->_sessions.emplace(key, Entry{.peer = from, .lastSeen = now}).first;
        } else {
            found->second = Entry{.peer = from, .lastSeen = now};
        }
        return encodeSessionPacket(PacketType::ConnectAck, from, found->second.nextSeq++);
    }
    const auto found = this->_sessions.find(from.toString());

    if (found == this->_sessions.end()) {
        return std::nullopt;
    }
    found->second.lastSeen = now;
    // The peer times out when the server stays silent: answer its heartbeats,
    // so it keeps hearing from the server while the game has nothing to send.
    if (packet.type == PacketType::Heartbeat) {
        return encodeSessionPacket(PacketType::Heartbeat, from, found->second.nextSeq++);
    }
    // Pong echoes the ping cookie untouched: the client times the round trip.
    if (packet.type == PacketType::Ping) {
        return encodeSessionPacket(PacketType::Pong, from, found->second.nextSeq++, packet.payload);
    }
    return std::nullopt;
}

std::vector<Endpoint> SessionTable::sweep(std::chrono::steady_clock::time_point now)
{
    std::vector<Endpoint> gone;

    for (auto it = this->_sessions.begin(); it != this->_sessions.end();) {
        if (now - it->second.lastSeen >= this->_timeout) {
            gone.push_back(it->second.peer);
            it = this->_sessions.erase(it);
        } else {
            ++it;
        }
    }
    return gone;
}

std::optional<std::uint32_t> SessionTable::nextSequence(const Endpoint &peer)
{
    const auto found = this->_sessions.find(peer.toString());

    if (found == this->_sessions.end()) {
        return std::nullopt;
    }
    return found->second.nextSeq++;
}

bool SessionTable::isConnected(const Endpoint &peer) const
{
    return this->_sessions.contains(peer.toString());
}

std::size_t SessionTable::getSize(void) const noexcept
{
    return this->_sessions.size();
}

} // namespace younity::network

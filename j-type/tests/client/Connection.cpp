/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Connection
** File description:
** Unit tests of the link to the server
*/

#include <criterion/criterion.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "jtype/common/Messages.hpp"
#include "network/Connection.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/Packet.hpp"
#include "younity/network/Session.hpp"
#include "younity/network/SessionTable.hpp"
#include "younity/network/UdpPump.hpp"

using jtype::Connection;
using jtype::decodeMessage;
using jtype::encodeMessage;
using jtype::FullMessage;
using jtype::JoinMessage;
using jtype::Message;
using jtype::WelcomeMessage;
using younity::network::Endpoint;

namespace {

// The duration of a step of the client, in seconds
constexpr float STEP = 0.016F;

// A timeout short enough for the tests to give up fast, many heartbeats long so a healthy session never drops
constexpr younity::network::SessionConfig SHORT_SESSION{
    .heartbeatInterval = std::chrono::milliseconds(20), .timeout = std::chrono::milliseconds(200)};

// A server on the loopback: it opens the sessions and answers each Join with its answer. Without an answer it stays
// silent, like an address where no server runs.
class FakeServer
{
public:
    explicit FakeServer(std::optional<Message> answer)
        : _answer(std::move(answer))
    {
        if (!this->_pump.bind(Endpoint::loopback(0)) || !this->_pump.start()) {
            throw std::runtime_error("cannot open the socket of the fake server");
        }
    }

    [[nodiscard]] Endpoint getEndpoint(void) const
    {
        return Endpoint::loopback(this->_pump.getLocalEndpoint()->getPort());
    }

    void serve(void)
    {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

        for (auto datagram = this->_pump.tryReceive(); datagram; datagram = this->_pump.tryReceive()) {
            const auto packet = younity::network::decodePacket(datagram->bytes);

            if (!this->_answer || !packet) {
                continue;
            }
            this->_sequences.push_back(packet->seq);
            if (const auto reply = this->_sessions.onPacket(datagram->from, *packet, now)) {
                (void)this->_pump.sendTo(reply->to, reply->bytes);
            }
            const std::expected<Message, jtype::DecodeError> message = decodeMessage(*packet);

            if (message && std::holds_alternative<JoinMessage>(*message)) {
                (void)this->_pump.sendTo(datagram->from, encodeMessage(*this->_answer, *this->_sessions.nextSequence(datagram->from)));
            }
        }
    }

    // The sequence numbers of the packets received from the client, in their order
    [[nodiscard]] const std::vector<std::uint32_t> &getSequences(void) const noexcept
    {
        return this->_sequences;
    }

private:
    younity::network::UdpPump _pump;
    younity::network::SessionTable _sessions;
    std::optional<Message> _answer;
    std::vector<std::uint32_t> _sequences;
};

// Run the steps of the client, as Client::tick does, until the connection gives up, and get its reason
std::string runUntilFailure(Connection &connection, FakeServer &server)
{
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

    while (std::chrono::steady_clock::now() < deadline) {
        server.serve();
        try {
            (void)connection.receive();
        } catch (const std::runtime_error &error) {
            return error.what();
        }
        if (connection.getSlot()) {
            (void)connection.sendInputs(0);
        } else {
            connection.join(STEP);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return "still connected";
}

} // namespace

Test(client_connection, numbers_its_packets_with_its_session)
{
    FakeServer server(WelcomeMessage{0});
    Connection connection(server.getEndpoint(), SHORT_SESSION);
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    std::size_t inputs = 0;

    // The handshake, the Join and the inputs: session packets and game packets in one sequence
    while (inputs < 3 && std::chrono::steady_clock::now() < deadline) {
        server.serve();
        (void)connection.receive();
        if (connection.getSlot()) {
            (void)connection.sendInputs(0);
            ++inputs;
        } else {
            connection.join(STEP);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    connection.leave();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    server.serve();

    const std::vector<std::uint32_t> &sequences = server.getSequences();

    cr_assert_eq(inputs, 3U);
    for (std::size_t index = 1; index < sequences.size(); ++index) {
        cr_assert_eq(sequences[index], sequences[index - 1] + 1);
    }
}

Test(client_connection, silent_server_is_unreachable)
{
    FakeServer server(std::nullopt);
    Connection connection(server.getEndpoint(), SHORT_SESSION);
    const std::string reason = runUntilFailure(connection, server);

    cr_assert_str_eq(reason.c_str(), "the server is unreachable");
}

Test(client_connection, full_server_gives_no_slot)
{
    FakeServer server(FullMessage{});
    Connection connection(server.getEndpoint(), SHORT_SESSION);
    const std::string reason = runUntilFailure(connection, server);

    cr_assert_str_eq(reason.c_str(), "the server is full");
    cr_assert_not(connection.getSlot().has_value());
}

Test(client_connection, server_silent_after_the_welcome_is_lost)
{
    FakeServer server(WelcomeMessage{2});
    Connection connection(server.getEndpoint(), SHORT_SESSION);
    const std::string reason = runUntilFailure(connection, server);

    cr_assert_str_eq(reason.c_str(), "the connection to the server was lost");
    cr_assert(connection.getSlot() == 2);
}

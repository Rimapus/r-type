/*
** EPITECH PROJECT, 2026
** j-type/server/src/Server
** File description:
** Server source file
*/

#include "Server.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <expected>
#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>

#include "Signals.hpp"
#include "jtype/common/Combat.hpp"
#include "jtype/common/EntityType.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Log.hpp"
#include "jtype/common/Paths.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/Waves.hpp"
#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Explosion.hpp"
#include "jtype/common/components/Health.hpp"
#include "jtype/common/components/Lifetime.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Obstacle.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/components/PowerUp.hpp"
#include "jtype/common/components/Score.hpp"
#include "jtype/common/systems/BossSystem.hpp"
#include "jtype/common/systems/BoundsSystem.hpp"
#include "jtype/common/systems/EnemyMovementSystem.hpp"
#include "jtype/common/systems/EnemyShootSystem.hpp"
#include "jtype/common/systems/InvulnerabilitySystem.hpp"
#include "jtype/common/systems/LifetimeSystem.hpp"
#include "jtype/common/systems/PlayerMovementSystem.hpp"
#include "jtype/common/systems/ShootSystem.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/ecs/systems/MovementSystem.hpp"
#include "younity/logger/Global.hpp"
#include "younity/network/Packet.hpp"
#include "younity/physics/components/Hitbox.hpp"
#include "younity/physics/systems/CollisionSystem.hpp"
#include "younity/tools/Commands.hpp"
#include "younity/tools/Global.hpp"

namespace jtype {

using younity::ecs::Entity;
using younity::ecs::Transform;
using younity::ecs::Velocity;
using younity::network::Endpoint;
using younity::physics::Hitbox;

namespace {

// The datagrams handled between two steps: the pump queue holds more, so a flood waits for the next frames instead of
// holding the current one
constexpr std::size_t MAX_DATAGRAMS_PER_STEP = 256;

// The events queued for one step: past that a notification is dropped, and the next snapshot or scoreboard still
// carries the change it announced
constexpr std::size_t MAX_EVENTS_PER_STEP = 256;

/**
 * @brief Read the waves of the game, next to the executable.
 *
 * @return The waves of assets/data/waves.txt.
 *
 * @throw std::runtime_error if the file is missing or wrong.
 */
std::vector<Wave> readWaveFile(void)
{
    auto waves = loadWaves(executableDir() / "assets" / "data" / "waves.txt");

    if (!waves) {
        throw std::runtime_error(waves.error());
    }
    return *waves;
}

} // namespace

Server::Server(std::uint16_t port, Difficulty difficulty)
    : younity::Application(TICK_STEP),
      _difficulty(difficulty),
      _rules(getDifficultyRules(difficulty)),
      _waveFile(readWaveFile()),
      _waves(this->_waveFile, std::random_device{}())
{
    if (!this->_pump.bind(port) || !this->_pump.start()) {
        throw std::runtime_error(std::format("cannot listen on the port {}", port));
    }
    this->_registry.addSystem<PlayerMovementSystem, Player, Transform>();
    this->_registry.addSystem<ShootSystem, Player, Transform>();
    this->_registry.addSystem<EnemyMovementSystem, Enemy, Transform, Velocity>();
    this->_registry.addSystem<EnemyShootSystem, Enemy, Transform>(this->_rules);
    this->_registry.addSystem<BossSystem, Boss, Transform>(this->_rules);
    this->_registry.addSystem<InvulnerabilitySystem, Player>();
    this->_registry.addSystem<LifetimeSystem, Lifetime>();
    this->_registry.addSystem<younity::ecs::MovementSystem, Transform, Velocity>();
    this->_registry.addSystem<BoundsSystem, Transform>();
    this->_registry.addSystem<younity::physics::CollisionSystem, Transform, Hitbox>(this->_bus);
    this->_bus.subscribe<younity::physics::CollisionEvent>([this](const younity::physics::CollisionEvent &event) {
        this->onCollision(event);
    });
    spawnTerrain(this->_registry);
    younity::tools::registerCommands(younity::tools::console(), younity::tools::metrics(), younity::logger::registry());
    registerCommands(younity::tools::console(), *this);
    logger().info("Listening on the port {}, difficulty {}", port, getDifficultyName(difficulty));
}

Server::~Server(void)
{
    removeCommands(younity::tools::console());
}

void Server::onInput(void)
{
    if (stopRequested()) {
        this->stop();
        return;
    }
    this->receive();
    this->runCommands();
    // The loop runs a frame as fast as it can and a server has nothing to do between two steps: leave the processor
    // to the other threads, like a client left to the vertical sync of its window
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void Server::onFixedUpdate(float step)
{
    if (this->_gameOver) {
        *this->_gameOver -= step;
        if (*this->_gameOver <= 0.0F) {
            this->restart();
        }
    } else if (this->hasClients()) {
        // An empty server waits: the first players to join find the waves at their start
        this->_registry.updateSystems(step);
        this->_waves.update(this->_registry, step, this->countShips());
        if (this->updatePlayers()) {
            this->_gameOver = GAME_OVER_DELAY;
            logger().info("Game over");
        } else if (this->_waves.isWon()) {
            this->_gameOver = GAME_OVER_DELAY;
            this->_scoreboard.victory = true;
            logger().info("Victory");
        }
    }
    this->_scoreboard.gameOver = this->_gameOver.has_value();
    this->broadcastSnapshot(this->_tick);
    this->broadcastEvents();
    this->broadcastScoreboard(this->_tick);
    if (const std::optional<TrafficRates> rates = this->_traffic.update(std::chrono::steady_clock::now())) {
        const auto clients = std::ranges::count_if(this->_clients, [](const std::optional<Client> &client) { return client.has_value(); });

        logger().info("Traffic with {} clients: {:.0f} B/s sent, {:.0f} B/s received", clients, rates->bytesSent, rates->bytesReceived);
    }
    ++this->_tick;
}

void Server::onStop(void)
{
    logger().info("Stopped");
}

void Server::runCommands(void)
{
    for (std::optional<std::string> line = this->_input.takeLine(); line; line = this->_input.takeLine()) {
        if (line->find_first_not_of(" \t\r") == std::string::npos) {
            continue;
        }
        const std::expected<std::string, younity::tools::ConsoleError> answer = younity::tools::console().execute(*line);

        if (answer) {
            logger().info("{}", *answer);
        } else {
            logger().warn("{}", answer.error().message);
        }
    }
}

std::vector<PlayerInfo> Server::getPlayers(void) const
{
    std::vector<PlayerInfo> players;

    for (std::uint8_t slot = 0; slot < MAX_PLAYERS; ++slot) {
        const PlayerStatus &status = this->_scoreboard.players[slot];
        const std::optional<Client> &client = this->_clients[slot];

        players.push_back(PlayerInfo{slot, status.state, client ? std::optional(client->endpoint) : std::nullopt, status.score,
                                     status.lives, client ? client->lastInput : 0});
    }
    return players;
}

bool Server::kickPlayer(std::uint8_t slot)
{
    if (slot >= MAX_PLAYERS || !this->_clients[slot]) {
        return false;
    }
    const Endpoint endpoint = this->_clients[slot]->endpoint;

    this->_kicked.push_back(endpoint);
    this->onLeave(endpoint);
    logger().info("Player {} kicked", slot + 1);
    return true;
}

void Server::restartGame(void)
{
    this->restart();
}

WaveProgress Server::getWaves(void) const
{
    return WaveProgress{this->_waves.getStartedWaves(), this->_waveFile.size()};
}

void Server::stopServer(void)
{
    this->stop();
}

Difficulty Server::getDifficulty(void) const
{
    return this->_difficulty;
}

void Server::setDifficulty(Difficulty difficulty)
{
    this->_difficulty = difficulty;
    this->_rules = getDifficultyRules(difficulty);
}

bool Server::updatePlayers(void)
{
    bool anyone = false;
    bool everyoneOut = true;

    for (std::size_t slot = 0; slot < MAX_PLAYERS; ++slot) {
        std::optional<Client> &client = this->_clients[slot];
        PlayerStatus &status = this->_scoreboard.players[slot];

        if (!client) {
            continue;
        }
        anyone = true;
        // The Player is removed with the last life, so a new entity with the same id is not taken for the ship
        if (client->ship && (!this->_registry.isAlive(*client->ship) || !this->_registry.hasComponent<Player>(*client->ship))) {
            client->ship.reset();
            status.state = SlotState::Out;
            status.lives = 0;
            logger().info("Player {} is out", slot + 1);
        }
        if (client->ship) {
            const Player &player = this->_registry.getComponent<Player>(*client->ship);

            status = PlayerStatus{SlotState::Playing, this->_registry.getComponent<Score>(*client->ship).points, player.lives,
                                  player.weapon};
            everyoneOut = false;
        }
    }
    return anyone && everyoneOut;
}

std::size_t Server::countShips(void) const
{
    return static_cast<std::size_t>(std::ranges::count_if(this->_clients, [](const std::optional<Client> &client) {
        return client && client->ship;
    }));
}

bool Server::hasClients(void) const
{
    return std::ranges::any_of(this->_clients, [](const std::optional<Client> &client) {
        return client.has_value();
    });
}

void Server::restart(void)
{
    // Every entity of the game has a Transform
    for (const Entity entity : this->_registry.getEntitiesWith<Transform>()) {
        this->_registry.destroyEntity(entity);
    }
    this->_scoreboard = ScoreboardMessage{};
    for (std::uint8_t slot = 0; slot < MAX_PLAYERS; ++slot) {
        if (this->_clients[slot]) {
            this->_clients[slot]->ship = spawnPlayer(this->_registry, slot, this->_rules.lives);
            this->_scoreboard.players[slot] = PlayerStatus{SlotState::Playing, 0, this->_rules.lives};
        }
    }
    // The ceiling and the floor have a Transform: destroyed with the rest of the game above
    spawnTerrain(this->_registry);
    this->_waves = WaveManager(this->_waveFile, std::random_device{}());
    this->_gameOver.reset();
    logger().info("New game");
}

void Server::broadcastScoreboard(std::uint32_t tick)
{
    this->_scoreboard.tick = tick;
    this->broadcast(this->_scoreboard);
}

void Server::receive(void)
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    for (std::size_t count = 0; count < MAX_DATAGRAMS_PER_STEP; ++count) {
        const std::optional<younity::network::ReceivedDatagram> datagram = this->_pump.tryReceive();

        if (!datagram) {
            break;
        }
        // A kicked client is not answered anymore: its session times out, and it leaves like a lost connection
        if (std::ranges::find(this->_kicked, datagram->from) != this->_kicked.end()) {
            continue;
        }
        const auto packet = younity::network::decodePacket(datagram->bytes);

        this->_traffic.addReceived(datagram->bytes.size());
        if (!packet) {
            this->logReject(younity::network::toString(packet.error()), now);
            continue;
        }
        if (const auto answer = this->_sessions.onPacket(datagram->from, *packet, now)) {
            this->sendBytes(answer->to, answer->bytes);
        }
        // Only a client with a session joins, so the sweep notices when it goes silent
        if (!this->_sessions.isConnected(datagram->from)) {
            continue;
        }
        // A session packet carries no game message: decoding it would count every heartbeat as garbage
        if (packet->type != younity::network::PacketType::GameData) {
            continue;
        }
        const std::expected<Message, DecodeError> message = decodeMessage(*packet);

        if (!message) {
            this->logReject(toString(message.error()), now);
            continue;
        }
        if (std::holds_alternative<JoinMessage>(*message)) {
            this->onJoin(datagram->from);
        } else if (std::holds_alternative<LeaveMessage>(*message)) {
            this->onLeave(datagram->from);
        } else if (const auto *input = std::get_if<InputMessage>(&*message)) {
            this->applyInput(datagram->from, *input);
        }
    }
    for (const Endpoint &silent : this->_sessions.sweep(now)) {
        this->onLeave(silent);
        std::erase(this->_kicked, silent);
    }
}

void Server::logReject(std::string_view reason, std::chrono::steady_clock::time_point now)
{
    const std::string line = this->_drops.reject(reason, now);

    if (!line.empty()) {
        logger().warn("{}", line);
    }
}

void Server::onJoin(const Endpoint &from)
{
    std::optional<std::uint8_t> slot = this->findSlot(from);

    if (!slot) {
        for (std::uint8_t free = 0; free < MAX_PLAYERS; ++free) {
            if (!this->_clients[free]) {
                slot = free;
                break;
            }
        }
        // Told at each Join, so a lost answer is made up by the next Join
        if (!slot) {
            this->send(from, FullMessage{});
            return;
        }
        this->_clients[*slot] = Client{from, spawnPlayer(this->_registry, *slot, this->_rules.lives), InputGate{}};
        this->_scoreboard.players[*slot] = PlayerStatus{SlotState::Playing, 0, this->_rules.lives};
        logger().info("Player {} joined from {}", *slot + 1, from.toString());
    }
    // A Join starts a new stream of inputs: the client may have restarted, so its steps start over, and the snapshots
    // stop giving back an input of the old stream
    this->_clients[*slot]->inputs = InputGate{};
    this->_clients[*slot]->lastInput = 0;
    this->send(from, WelcomeMessage{*slot});
}

void Server::onLeave(const Endpoint &from)
{
    const std::optional<std::uint8_t> slot = this->findSlot(from);

    // A client that sent Leave is swept later too, and a client that never joined has nothing to free
    if (!slot) {
        return;
    }
    const std::optional<Entity> ship = this->_clients[*slot]->ship;

    if (ship && this->_registry.isAlive(*ship) && this->_registry.hasComponent<Player>(*ship)) {
        this->_registry.destroyEntity(*ship);
    }
    this->_clients[*slot].reset();
    this->_scoreboard.players[*slot].state = SlotState::Left;
    this->_scoreboard.players[*slot].lives = 0;
    this->queueEvent(PlayerDisconnectedMessage{this->_tick, *slot});
    logger().info("Player {} left", *slot + 1);
    if (!this->hasClients()) {
        this->restart();
    }
}

void Server::applyInput(const Endpoint &from, const InputMessage &message)
{
    const std::optional<std::uint8_t> slot = this->findSlot(from);

    if (!slot) {
        return;
    }
    Client &client = *this->_clients[*slot];

    // A duplicated or reordered datagram must not bring back keys the client already released
    if (!client.inputs.accept(message.tick)) {
        return;
    }
    client.lastInput = message.tick;
    if (client.ship) {
        this->_registry.getComponent<Player>(*client.ship).inputs = message.inputs;
    }
}

void Server::onCollision(const younity::physics::CollisionEvent &event)
{
    std::vector<Death> deaths;
    std::vector<PickUp> pickUps;

    resolveCollision(this->_registry, event, &deaths, &pickUps);
    for (const Death &death : deaths) {
        this->queueEvent(DeathEventMessage{this->_tick, death.entity.getId(), death.killer.value_or(NO_PLAYER)});
    }
    for (const PickUp &pickUp : pickUps) {
        this->queueEvent(PickUpEventMessage{this->_tick, pickUp.slot, pickUp.type});
    }
}

namespace {

/**
 * @brief Get the type a Bydo has in the snapshots, so the client draws its sprite.
 *
 * @param type The type of the Bydo
 *
 * @return The type of the entity.
 */
EntityType toEntityType(EnemyType type)
{
    switch (type) {
    case EnemyType::Jelly:
        return EntityType::Jelly;
    case EnemyType::SentryDrone:
        return EntityType::SentryDrone;
    case EnemyType::Blobling:
        break;
    }
    return EntityType::Blobling;
}

/**
 * @brief Get the type a missile of the Bydos or of the boss has in the snapshots.
 *
 * @param kind What fired it
 *
 * @return The type of the entity.
 */
EntityType toEntityType(MissileKind kind)
{
    switch (kind) {
    case MissileKind::BossOrb:
        return EntityType::IncubatorOrb;
    case MissileKind::BossSpit:
        return EntityType::IncubatorSpit;
    // A pellet always has a shooter: it is a missile of a player
    case MissileKind::Pellet:
    case MissileKind::Orb:
        break;
    }
    return EntityType::EnemyMissile;
}

/**
 * @brief Get the type a power-up has in the snapshots.
 *
 * @param type What it gives
 *
 * @return The type of the entity.
 */
EntityType toEntityType(PowerUpType type)
{
    switch (type) {
    case PowerUpType::Shield:
        return EntityType::ShieldPowerUp;
    case PowerUpType::SpreadShot:
        return EntityType::SpreadShotPowerUp;
    case PowerUpType::ExtraLife:
        break;
    }
    return EntityType::ExtraLifePowerUp;
}

/**
 * @brief Get the type a wall or a zapper has in the snapshots: a block cracks once it took a hit.
 *
 * @param registry The registry of the game
 * @param obstacle The obstacle, not the ceiling nor the floor
 *
 * @return The type of the entity.
 */
EntityType getObstacleType(const younity::ecs::Registry &registry, Entity obstacle)
{
    switch (registry.getComponent<Obstacle>(obstacle).kind) {
    case ObstacleKind::ZapperNode:
        return EntityType::ZapperNode;
    case ObstacleKind::ZapperBeam:
        return EntityType::ZapperBeam;
    case ObstacleKind::Block:
    case ObstacleKind::Terrain:
        break;
    }
    const Health &health = registry.getComponent<Health>(obstacle);

    return health.current < health.max ? EntityType::CrackedBlock : EntityType::Block;
}

/**
 * @brief Get the type a part of the boss has in the snapshots: its eye turns angry once it lost half of its health.
 *
 * @param registry The registry of the game
 * @param part The part of the boss
 *
 * @return The type of the entity.
 */
EntityType getBossPartType(const younity::ecs::Registry &registry, Entity part)
{
    if (registry.getComponent<Boss>(part).part == BossPart::Body) {
        return EntityType::IncubatorBody;
    }
    const Health &health = registry.getComponent<Health>(part);

    return health.current * 2 <= health.max ? EntityType::IncubatorEyeAngry : EntityType::IncubatorEye;
}

} // namespace

void Server::broadcastSnapshot(std::uint32_t tick)
{
    SnapshotMessage snapshot;

    snapshot.tick = tick;
    for (const Entity entity : this->_registry.getEntitiesWith<Transform>()) {
        const younity::Vector2f &position = this->_registry.getComponent<Transform>(entity).position;
        EntityType type = EntityType::Blobling;
        std::uint8_t health = 0;

        // The clients draw the ceiling and the floor themselves, scrolling with the stage
        if (this->_registry.hasComponent<Obstacle>(entity) && this->_registry.getComponent<Obstacle>(entity).kind == ObstacleKind::Terrain) {
            continue;
        }
        if (this->_registry.hasComponent<Explosion>(entity)) {
            type = EntityType::Explosion;
        } else if (this->_registry.hasComponent<BossWreck>(entity)) {
            type = EntityType::IncubatorWreck;
        } else if (this->_registry.hasComponent<Boss>(entity)) {
            type = getBossPartType(this->_registry, entity);
        } else if (this->_registry.hasComponent<Enemy>(entity)) {
            type = toEntityType(this->_registry.getComponent<Enemy>(entity).type);
        } else if (this->_registry.hasComponent<Player>(entity)) {
            type = static_cast<EntityType>(this->_registry.getComponent<Player>(entity).slot);
        } else if (this->_registry.hasComponent<Missile>(entity)) {
            const Missile &missile = this->_registry.getComponent<Missile>(entity);

            if (!missile.shooter) {
                type = toEntityType(missile.kind);
            } else {
                type = missile.kind == MissileKind::Pellet ? getPelletType(*missile.shooter) : getMissileType(*missile.shooter);
            }
        } else if (this->_registry.hasComponent<PowerUp>(entity)) {
            type = toEntityType(this->_registry.getComponent<PowerUp>(entity).type);
        } else if (this->_registry.hasComponent<Obstacle>(entity)) {
            type = getObstacleType(this->_registry, entity);
        }
        if (this->_registry.hasComponent<Health>(entity)) {
            health = static_cast<std::uint8_t>(std::clamp(this->_registry.getComponent<Health>(entity).current, 0, 255));
        }
        snapshot.entities.push_back(EntityState{entity.getId(), type, position.x, position.y, health});
    }
    // The entities that appeared or went since the previous step are the spawns and destructions of this one
    for (Message &event : this->_tracker.update(tick, snapshot.entities, this->_scoreboard)) {
        this->queueEvent(std::move(event));
    }
    // A snapshot too large for a datagram travels in several chunks, each sent to every client with the last input of
    // that client: each client predicts its own ship from the last input the ship moved with
    for (SnapshotMessage &chunk : splitSnapshot(std::move(snapshot))) {
        for (const std::optional<Client> &client : this->_clients) {
            if (client) {
                chunk.lastInput = client->lastInput;
                this->send(client->endpoint, chunk);
            }
        }
    }
}

void Server::broadcastEvents(void)
{
    for (const Message &event : this->_events) {
        this->broadcast(event);
    }
    this->_events.clear();
}

void Server::queueEvent(Message event)
{
    if (this->_events.size() < MAX_EVENTS_PER_STEP) {
        this->_events.push_back(std::move(event));
    }
}

std::optional<std::uint8_t> Server::findSlot(const Endpoint &endpoint) const
{
    for (std::uint8_t slot = 0; slot < MAX_PLAYERS; ++slot) {
        if (this->_clients[slot] && this->_clients[slot]->endpoint == endpoint) {
            return slot;
        }
    }
    return std::nullopt;
}

void Server::send(const Endpoint &to, const Message &message)
{
    // Numbered by the session of the client, which numbers its own answers too: the client sees one sequence
    const std::optional<std::uint32_t> seq = this->_sessions.nextSequence(to);

    if (seq) {
        this->sendBytes(to, encodeMessage(message, *seq));
    }
}

void Server::sendBytes(const Endpoint &to, std::span<const std::byte> bytes)
{
    // A datagram the full queue dropped never reaches the network
    if (this->_pump.sendTo(to, bytes)) {
        this->_traffic.addSent(bytes.size());
    }
}

void Server::broadcast(const Message &message)
{
    for (const std::optional<Client> &client : this->_clients) {
        if (client) {
            this->send(client->endpoint, message);
        }
    }
}

} // namespace jtype

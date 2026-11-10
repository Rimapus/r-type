/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/GameScene
** File description:
** GameScene source file
*/

#include "scenes/GameScene.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Input.hpp"
#include "jtype/common/Log.hpp"
#include "jtype/common/Messages.hpp"
#include "render/HudLayout.hpp"
#include "render/Screen.hpp"
#include "render/Sprites.hpp"
#include "scenes/GameOverScene.hpp"
#include "scenes/MenuScene.hpp"
#include "younity/render/components/Animation.hpp"
#include "younity/render/systems/AnimationSystem.hpp"

namespace jtype {

GameScene::GameScene(Context &context)
    : _context(context),
      _connection(context.server),
      _world(this->getRegistry(), context.assets, context.settings.reduceFlashes),
      _starfield(this->getRegistry()),
      _terrain(this->getRegistry(), TERRAIN_LAYER),
      _hud(this->getRegistry(), context.assets),
      _texts(this->getRegistry(), context.assets)
{
    this->getRegistry().addSystem<younity::render::AnimationSystem, younity::render::Animation>(this->getEventBus());
    for (const StarfieldLayer &layer : STARFIELD_LAYERS) {
        this->_starfield.addLayer(context.assets.loadTexture(layer.path), SCROLL_SPEED * layer.parallax);
    }
    this->_starfield.setColor(context.settings.highContrast ? DIMMED_STARFIELD : younity::Color{});
    for (const TerrainLayer &layer : TERRAIN_LAYERS) {
        this->_terrain.addLayer(context.assets.loadTexture(layer.path), STAGE_SPEED, layer.top);
    }
    logger().info("Joining {}", this->_connection.getServer().toString());
}

GameScene::~GameScene(void)
{
    this->_connection.leave();
}

void GameScene::update(float dt)
{
    if (this->_context.controls.isMenuPressed(MenuAction::Back)) {
        this->_context.scenes.changeScene<MenuScene>(this->_context);
        return;
    }

    Received received;

    try {
        received = this->_connection.receive();
    } catch (const std::runtime_error &error) {
        // Back to the main menu, where PLAY tries again
        this->_context.scenes.changeScene<MenuScene>(this->_context, error.what());
        return;
    }

    const std::optional<std::uint8_t> slot = this->_connection.getSlot();
    const ScoreboardMessage &scoreboard = this->_connection.getScoreboard();

    for (SnapshotMessage &snapshot : received.snapshots) {
        if (slot) {
            this->_prediction.correct(snapshot, *slot);
        }
        // Dropped while the editor pauses the game, so the buffer does not grow: Play goes back to the live game
        if (!this->_context.paused) {
            this->_snapshots.push(std::move(snapshot));
        }
    }
    for (Message &event : received.events) {
        if (!this->_context.paused) {
            this->_events.push(std::move(event));
        }
    }
    // Moved before the world is drawn, so the ship answers the keys in this very frame
    if (slot) {
        // Auto fire holds the shoot key for the player
        const std::uint8_t inputs = this->_context.controls.read() | (this->_context.settings.autoFire ? input::SHOOT : 0U);

        this->_prediction.move(this->_connection.sendInputs(inputs), inputs, dt);
    } else {
        this->_connection.join(dt);
    }
    // The session lives on above, so the server keeps the player while the world below stands still
    if (this->_context.paused) {
        return;
    }
    if (std::optional<SnapshotMessage> drawn = this->_snapshots.step()) {
        if (slot) {
            this->_prediction.place(*drawn, *slot);
        }
        // Before the world moves on: it still has the ship a death of these steps names
        this->playEvents(this->_events.release(drawn->tick), scoreboard);
        this->_world.apply(*drawn, slot);
        this->showBoss(*drawn);
        this->_shown = std::move(*drawn);
    }
    this->_hud.show(scoreboard, slot);
    this->_context.audio.setGameOver(scoreboard.gameOver);
    // Every list moves on at each step: none is skipped when another one already changed
    const bool captions = this->_captions.update(dt);
    const bool notices = this->_notices.update(dt);
    const bool popups = this->_popups.update(dt);

    if (captions || notices || popups) {
        this->_texts.clear();
        for (const std::vector<HudText> &texts : {this->_captions.layout(), this->_notices.layout(), this->_popups.layout()}) {
            for (const HudText &text : texts) {
                this->_texts.write(text);
            }
        }
    }
    younity::ecs::Scene::update(dt);
    this->_starfield.scroll(dt);
    this->_terrain.scroll(dt);
    if (slot && scoreboard.players[*slot].state != SlotState::Free) {
        if (!scoreboard.gameOver) {
            this->_playing = true;
        } else if (this->_playing) {
            this->_context.scenes.changeScene<GameOverScene>(this->_context, scoreboard, *slot);
        }
    }
}

void GameScene::showPickUp(const PickUpEventMessage &pickUp)
{
    const auto ship = std::ranges::find_if(this->_shown.entities, [&pickUp](const EntityState &entity) {
        return getPlayerSlot(entity.type) == pickUp.slot;
    });

    if (ship != this->_shown.entities.end()) {
        this->_popups.add(std::string(getPowerUpName(pickUp.powerUp)), {ship->x, ship->y}, PLAYER_COLORS[pickUp.slot]);
    }
}

void GameScene::showBoss(const SnapshotMessage &snapshot)
{
    const std::optional<int> health = getBossHealth(snapshot);

    // The bar is full at the health the boss came with, which depends on the players of that moment
    this->_bossMaxHealth = health ? std::max(this->_bossMaxHealth, *health) : 0;
    this->_hud.showBoss(health && this->_bossMaxHealth > 0 ? std::optional(std::pair(*health, this->_bossMaxHealth)) : std::nullopt);
}

void GameScene::playEvents(const std::vector<Message> &events, const ScoreboardMessage &scoreboard)
{
    std::vector<Sound> sounds;
    // The entity each player destroyed last in these steps: its points show where it was
    std::array<std::optional<std::uint32_t>, MAX_PLAYERS> destroyed{};

    for (const Message &event : events) {
        if (const std::optional<Sound> sound = getEventSound(event, this->_shown, scoreboard)) {
            sounds.push_back(*sound);
        }
        if (const auto *death = std::get_if<DeathEventMessage>(&event); death != nullptr && death->killer < MAX_PLAYERS) {
            destroyed[death->killer] = death->id;
        } else if (const auto *left = std::get_if<PlayerDisconnectedMessage>(&event)) {
            this->_notices.announce(std::format("P{} DISCONNECTED", left->slot + 1), PLAYER_COLORS[left->slot]);
        } else if (const auto *pickUp = std::get_if<PickUpEventMessage>(&event)) {
            this->showPickUp(*pickUp);
        } else if (const auto *score = std::get_if<ScoreEventMessage>(&event); score != nullptr && destroyed[score->slot]) {
            const auto entity = std::ranges::find(this->_shown.entities, *destroyed[score->slot], &EntityState::id);

            if (entity != this->_shown.entities.end()) {
                this->_popups.add(std::format("+{}", score->points), {entity->x, entity->y}, PLAYER_COLORS[score->slot]);
            }
        }
    }
    this->play(sounds);
}

void GameScene::play(const std::vector<Sound> &sounds)
{
    this->_context.audio.play(sounds);
    // Shown with the sound on too: a player hard of hearing can play with both
    if (this->_context.settings.captions) {
        this->_captions.hear(sounds);
    }
}

} // namespace jtype

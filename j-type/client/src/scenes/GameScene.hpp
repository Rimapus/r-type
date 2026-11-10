/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/GameScene
** File description:
** GameScene header file
*/

#pragma once

#include <vector>

#include "audio/SoundCues.hpp"
#include "network/Connection.hpp"
#include "render/Hud.hpp"
#include "render/Overlay.hpp"
#include "render/Starfield.hpp"
#include "scenes/Captions.hpp"
#include "scenes/Context.hpp"
#include "scenes/Notices.hpp"
#include "scenes/Popups.hpp"
#include "world/EventBuffer.hpp"
#include "world/RemoteWorld.hpp"
#include "world/ShipPrediction.hpp"
#include "world/SnapshotBuffer.hpp"
#include "younity/ecs/Scene.hpp"

namespace jtype {

/**
 * @brief A game on the server: it joins it, sends the inputs and draws the snapshots, two steps behind with the ship
 *        of the player ahead, and the last scoreboard received. Leaving the scene leaves the game.
 */
class GameScene : public younity::ecs::Scene
{
public:
    /**
     * @brief Open the socket: the server is joined from the first step.
     *
     * @param context What the scenes share
     *
     * @throw std::runtime_error if the socket cannot be opened or an asset is missing.
     */
    explicit GameScene(Context &context);

    /**
     * @brief Tell the server the player quits.
     */
    ~GameScene(void) override;

    /**
     * @brief Run one step: receive, send the inputs and move the ship of the player by them, move the drawn world a
     *        step, update the HUD and the health bar of the boss, play the sounds and show their captions, animate the
     *        sprites, scroll the starfield, the ceiling and the floor.
     *        Back, a full server or a lost server go back to the main menu, the end of the game to the game over
     *        screen.
     *
     * @param dt The duration of the step, in seconds
     */
    void update(float dt) override;

private:
    /**
     * @brief Play the events of the steps drawn, before the world drawn moves on to them: their sounds, the notice of a
     *        player who left, the points of a Bydo destroyed where it was, and the power-up a ship took over it.
     *
     * @param events The events released by the EventBuffer
     * @param scoreboard The last scoreboard
     */
    void playEvents(const std::vector<Message> &events, const ScoreboardMessage &scoreboard);

    /**
     * @brief Show what a ship took, over the ship, e.g. SHIELD.
     *
     * @param pickUp The power-up and the player who took it
     */
    void showPickUp(const PickUpEventMessage &pickUp);

    /**
     * @brief Show the health bar of the boss while it is in the world drawn, full at the health it came with.
     *
     * @param snapshot The world drawn
     */
    void showBoss(const SnapshotMessage &snapshot);

    /**
     * @brief Play sounds, and show their captions when the settings ask for them.
     *
     * @param sounds The sounds to play
     */
    void play(const std::vector<Sound> &sounds);

    Context &_context;
    Connection _connection;
    SnapshotBuffer _snapshots;
    ShipPrediction _prediction;
    RemoteWorld _world;
    Starfield _starfield;
    // The ceiling and the floor, not dimmed with the starfield: a ship that touches them loses a life
    Starfield _terrain;
    Hud _hud;
    EventBuffer _events;
    // The world drawn last, which the events of the next steps name
    SnapshotMessage _shown;
    Captions _captions;
    Notices _notices;
    Popups _popups;
    // The captions, the notices and the points, drawn again when one of them changes
    Overlay _texts;
    // The most health seen on the boss since it came, 0 without a boss
    int _bossMaxHealth = 0;
    // Set once the game is seen running: a client joining during the game over waits for the next game instead
    bool _playing = false;
};

} // namespace jtype

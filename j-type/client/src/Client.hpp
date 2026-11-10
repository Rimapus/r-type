/*
** EPITECH PROJECT, 2026
** j-type/client/src/Client
** File description:
** Client header file
*/

#pragma once

#include "audio/Audio.hpp"
#include "input/Controls.hpp"
#include "render/Assets.hpp"
#include "scenes/Context.hpp"
#include "settings/Settings.hpp"
#include "system/Process.hpp"
#include "younity/core/Application.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/SceneManager.hpp"
#include "younity/editor/AssetBrowser.hpp"
#include "younity/editor/ConsoleWindow.hpp"
#include "younity/editor/Editor.hpp"
#include "younity/editor/MetricsWindow.hpp"
#include "younity/editor/Workspace.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/render/Window.hpp"

namespace jtype {

/**
 * @brief The game client: the window, the keys, the sound and the settings, shared by its screens, the main menu,
 *        the settings, the game and the game over. DEBUG_LAYOUT_KEY shows the debug windows of the engine over them,
 *        EDITOR_LAYOUT_KEY its editor around them.
 */
class Client : public younity::Application
{
public:
    /**
     * @brief Open the window, load the assets and the settings, and start on the main menu.
     *
     * @param server The server PLAY joins
     *
     * @throw std::runtime_error if an asset is missing.
     */
    explicit Client(younity::network::Endpoint server);

    /**
     * @brief Remove the commands of the game from the console, which outlives the client.
     */
    ~Client(void) override;

protected:
    /**
     * @brief Read the events of the window, and stop the loop once the window is closed or QUIT is chosen.
     */
    void onInput(void) override;

    /**
     * @brief Run one step: read the keys if the layout gives them to the game, mute or unmute the sound, update the
     *        screen shown, which stands still while the editor pauses the game.
     *
     * @param step The duration of the step, in seconds
     *
     * @throw std::runtime_error if the socket of a game cannot be opened or an asset is missing.
     */
    void onFixedUpdate(float step) override;

    /**
     * @brief Draw the screen shown and the windows of the editor.
     *
     * @param alpha Unused: the game screen draws the world between two snapshots itself, with its SnapshotBuffer
     */
    void onRender(float alpha) override;

private:
    younity::network::Endpoint _server;
    younity::render::Window _window;
    Controls _controls;
    Assets _assets;
    Audio _audio;
    Settings _settings;
    bool _quit = false;
    // Read from the Play, Pause and Step buttons of the editor at each step
    bool _paused = false;
    // J-Climb, started by the Konami code of the main menu
    Process _jclimb;
    younity::ecs::ComponentRegistry _components;
    younity::editor::Editor _editor;
    younity::editor::ConsoleWindow _console;
    younity::editor::MetricsWindow _metrics;
    younity::editor::AssetBrowser _browser;
    younity::editor::Workspace _workspace;
    // Declared after what the scenes use, so the scenes are destroyed first
    younity::ecs::SceneManager _scenes;
    Context _context;
};

} // namespace jtype

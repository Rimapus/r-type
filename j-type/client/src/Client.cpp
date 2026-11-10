/*
** EPITECH PROJECT, 2026
** j-type/client/src/Client
** File description:
** Client source file
*/

#include "Client.hpp"

#include <chrono>
#include <thread>
#include <utility>

#include "input/Bindings.hpp"
#include "jtype/common/Constants.hpp"
#include "render/Screen.hpp"
#include "scenes/Commands.hpp"
#include "scenes/MenuScene.hpp"
#include "younity/ecs/Components.hpp"
#include "younity/editor/Layout.hpp"
#include "younity/logger/Global.hpp"
#include "younity/render/Components.hpp"
#include "younity/render/Renderer.hpp"
#include "younity/tools/Commands.hpp"
#include "younity/tools/Global.hpp"

namespace jtype {

Client::Client(younity::network::Endpoint server)
    : younity::Application(TICK_STEP),
      _server(std::move(server)),
      _window(static_cast<unsigned int>(WORLD_WIDTH * SCALE), static_cast<unsigned int>(WORLD_HEIGHT * SCALE), "J-Type"),
      _audio(_assets),
      _settings(loadSettings(getSettingsPath())),
      _editor(_components),
      _console(younity::tools::console(), younity::logger::registry()),
      _browser(_assets.getResources()),
      _workspace(_editor, _console, _metrics, _browser),
      _context{_scenes, _window, _assets, _audio, _controls, _settings, _server, _quit, _paused, _jclimb}
{
    // The components the inspector of the editor shows, the textures by their path in the assets
    younity::ecs::registerComponents(this->_components);
    younity::render::registerComponents(this->_components, this->_assets.getResources());
    // The world comes from the server: a scene file loaded under it would not match the entities the screens hold
    this->_editor.setSceneWindowShown(false);
    // The commands of the engine, of its editor and of the game, listed by help in the console of the layouts
    younity::tools::registerCommands(younity::tools::console(), younity::tools::metrics(), younity::logger::registry());
    this->_editor.registerCommands(younity::tools::console());
    registerCommands(younity::tools::console(), this->_context);
    this->_workspace.bindKey(younity::editor::Layout::Debug, DEBUG_LAYOUT_KEY);
    this->_workspace.bindKey(younity::editor::Layout::Editor, EDITOR_LAYOUT_KEY);
    this->_audio.setVolumes(this->_settings);
    this->_controls.apply(this->_settings.bindings);
    applyColorblind(this->_context);
    this->_scenes.changeScene<MenuScene>(this->_context);
}

Client::~Client(void)
{
    removeCommands(younity::tools::console());
}

void Client::onInput(void)
{
    this->_window.pollEvents();
    if (!this->_window.isOpen() || this->_quit) {
        this->stop();
    }
}

void Client::onFixedUpdate(float step)
{
    this->_controls.setEnabled(this->_workspace.hasGameInput());
    this->_controls.update();
    // Here and not in a scene, so M mutes the sound on every screen
    if (this->_controls.isMutePressed()) {
        this->_audio.toggleMute();
    }
    this->_paused = !this->_editor.shouldUpdate();
    this->_scenes.update(step);
}

void Client::onRender(float /* alpha */)
{
    this->_window.clear();
    // The main menu is shown from the first step
    if (this->_scenes.hasActiveScene()) {
        younity::render::drawEntities(this->_scenes.getActiveScene().getRegistry(), this->_window);
        this->_workspace.draw(this->_scenes.getActiveScene().getRegistry(), this->_window);
    }
    this->_window.display();
    // The window has no vertical sync: leave the processor to the network threads between two frames
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

} // namespace jtype

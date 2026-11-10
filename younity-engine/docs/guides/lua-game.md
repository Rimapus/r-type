# Write a game in Lua

This guide builds a small game whose rules are all in Lua: a title screen, a ship the player moves with the keyboard or a gamepad, a sound when it moves, and a best time kept between two runs. The C++ part is written once: it opens the window, connects the input and the audio to the scripts, and runs the engine. Everything else is in `assets/`, and changes while the game runs.

The [Scripting](../architecture/scripting.md) page describes each table the scripts use.

## The C++ part

The game is a `younity::Application`. It owns the modules, gives the scripts a `scripting::Host`, and does between two steps what the scripts asked: load another scene, or change the time scale. A game that needs more systems, like the movement or the collisions, adds them next to the `ScriptSystem`.

```cpp
// src/main.cpp
#include <optional>
#include <string>
#include <utility>

#include "younity/assets/ResourceManager.hpp"
#include "younity/audio/Music.hpp"
#include "younity/audio/SoundBuffer.hpp"
#include "younity/audio/SoundPlayer.hpp"
#include "younity/core/Application.hpp"
#include "younity/core/EventBus.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Components.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/SceneFile.hpp"
#include "younity/input/InputManager.hpp"
#include "younity/input/Names.hpp"
#include "younity/render/Components.hpp"
#include "younity/render/Font.hpp"
#include "younity/render/Renderer.hpp"
#include "younity/render/Texture.hpp"
#include "younity/render/Window.hpp"
#include "younity/scripting/Components.hpp"
#include "younity/scripting/Host.hpp"
#include "younity/scripting/components/Script.hpp"
#include "younity/scripting/systems/ScriptSystem.hpp"

using namespace younity;

class LuaGame : public Application
{
public:
    LuaGame(void)
        : Application(1.0F / 60.0F),
          _window(512, 512, "Lua game"),
          _resources("assets")
    {
        this->_resources.setLoader<render::Texture>(render::Texture::loadFromFile);
        this->_resources.setLoader<render::Font>(render::Font::loadFromFile);
        this->_resources.setLoader<audio::SoundBuffer>(audio::SoundBuffer::loadFromFile);
        ecs::registerComponents(this->_components);
        render::registerComponents(this->_components, this->_resources);
        scripting::registerComponents(this->_components);
        this->_registry.addSystem<scripting::ScriptSystem, scripting::Script>(this->_components, this->_bus,
                                                                              this->createHost(), "assets");
        this->_scene = "scenes/title.json";
    }

protected:
    void onInput(void) override
    {
        this->_window.pollEvents();
        if (!this->_window.isOpen()) {
            this->stop();
        }
    }

    void onFixedUpdate(float step) override
    {
        // A scene cannot be loaded while the systems update: the one a script asked for is loaded here
        if (this->_scene) {
            if (!ecs::loadScene(this->_registry, this->_components, "assets/" + *this->_scene)) {
                this->stop();
            }
            this->_scene.reset();
        }
        this->_input.update();
        this->_registry.updateSystems(step * this->_timeScale);
    }

    void onRender(float /* alpha */) override
    {
        this->_window.clear();
        render::drawEntities(this->_registry, this->_window);
        this->_window.display();
    }

private:
    scripting::Host createHost(void)
    {
        scripting::Host host{};

        host.actions.isPressed = [this](const std::string &action) { return this->_input.isActionPressed(action); };
        host.actions.isJustPressed = [this](const std::string &action) {
            return this->_input.isActionJustPressed(action);
        };
        host.actions.isJustReleased = [this](const std::string &action) {
            return this->_input.isActionJustReleased(action);
        };
        host.actions.bindKey = [this](const std::string &action, const std::string &name) {
            const std::optional<input::Key> key = input::parseKey(name);

            if (key) {
                this->_input.bindKey(action, *key);
            }
            return key.has_value();
        };
        host.actions.bindButton = [this](const std::string &action, unsigned int button) {
            this->_input.bindGamepadButton(action, button);
        };
        host.actions.bindAxis = [this](const std::string &action, const std::string &name, bool positive) {
            const std::optional<input::GamepadAxis> axis = input::parseGamepadAxis(name);

            if (axis) {
                this->_input.bindGamepadAxis(action, *axis,
                                             positive ? input::AxisDirection::Positive : input::AxisDirection::Negative);
            }
            return axis.has_value();
        };
        host.actions.unbind = [this](const std::string &action) { this->_input.unbind(action); };
        host.actions.getBindings = [this](const std::string &action) {
            return input::describe(this->_input.getBindings(action));
        };
        host.audio.play = [this](const std::string &path, float volume) {
            if (const auto sound = this->_resources.load<audio::SoundBuffer>(path)) {
                this->_sounds.play(*sound, volume);
            }
        };
        host.audio.playMusic = [this](const std::string &path, bool looping) {
            // The same music again goes on from where it was paused
            if (path != this->_musicPath) {
                auto music = audio::Music::openFromFile(this->_resources.getRoot() / path);

                if (!music) {
                    return;
                }
                this->_music = std::move(*music);
                this->_musicPath = path;
            }
            this->_music->setLooping(looping);
            this->_music->setVolume(this->_musicVolume);
            this->_music->play();
        };
        host.audio.pauseMusic = [this] {
            if (this->_music) {
                this->_music->pause();
            }
        };
        host.audio.stopMusic = [this] {
            if (this->_music) {
                this->_music->stop();
            }
        };
        host.audio.setVolume = [this](float volume) { this->_sounds.setVolume(volume); };
        host.audio.setMusicVolume = [this](float volume) {
            this->_musicVolume = volume;
            if (this->_music) {
                this->_music->setVolume(volume);
            }
        };
        host.game.loadScene = [this](const std::string &path) { this->_scene = path; };
        host.game.quit = [this] { this->stop(); };
        host.game.setTimeScale = [this](float scale) { this->_timeScale = scale; };
        host.saves = "saves";
        return host;
    }

    render::Window _window;
    assets::ResourceManager _resources;
    input::InputManager _input;
    audio::SoundPlayer _sounds;
    std::optional<audio::Music> _music;
    std::string _musicPath;
    float _musicVolume = 1.0F;
    ecs::ComponentRegistry _components;
    EventBus _bus;
    ecs::Registry _registry;
    std::optional<std::string> _scene;
    float _timeScale = 1.0F;
};

int main(void)
{
    LuaGame game;

    game.run();
    return 0;
}
```

This file does not change any more: the next steps are all in `assets/`.

## The scenes

A scene file holds one entity that runs a script; the script builds the rest. The title screen:

```json
{
  "format": "younity-scene",
  "version": 2,
  "entities": [
    { "id": 0, "components": { "Script": { "path": "scripts/title.lua" } } }
  ]
}
```

Save it as `assets/scenes/title.json`, and the same with `scripts/level.lua` as `assets/scenes/level.json`.

## The controls, in a module

The keys are bound in Lua, in a module the two scenes require. A module runs once, so the keys are bound once:

```lua
-- assets/scripts/controls.lua
-- Unbound first: saved again, the file binds its new keys in place of the old ones
for _, action in ipairs({ "left", "right", "start", "quit" }) do
    input.unbind(action)
end
input.bindKey("left", "Left")
input.bindKey("right", "Right")
input.bindAxis("left", "X", -1)
input.bindAxis("right", "X", 1)
input.bindKey("start", "Enter")
input.bindButton("start", 0)
input.bindKey("quit", "Escape")

return { start = input.getBindings("start") }
```

## The title screen

```lua
-- assets/scripts/title.lua
local controls = require("scripts.controls")
local Title = {}

function Title.start(entity)
    local best = storage.load("best")
    local text = ecs.create()

    ecs.add(text, "Transform")
    ecs.set(text, "Transform", { position = { x = 120, y = 220 } })
    ecs.add(text, "Text")
    ecs.set(text, "Text", {
        font = "fonts/font.ttf",
        string = "PRESS " .. controls.start[1] .. (best and ("\nBEST " .. best.time .. " S") or ""),
        characterSize = 24,
    })
    audio.playMusic("music/title.ogg")
end

function Title.update(entity, dt)
    if input.isJustPressed("start") then
        game.loadScene("scenes/level.json")
    elseif input.isJustPressed("quit") then
        game.quit()
    end
end

return Title
```

`controls.start[1]` is the first thing bound to `start`, `Enter` here: change the key in `controls.lua` and the title follows.

## The level

```lua
-- assets/scripts/level.lua
require("scripts.controls")

local SPEED = 200
local GOAL = 400
local Level = { time = 0 }

function Level.start(entity)
    Level.ship = ecs.create()
    ecs.add(Level.ship, "Transform")
    ecs.set(Level.ship, "Transform", { position = { x = 40, y = 240 } })
    ecs.add(Level.ship, "Sprite")
    ecs.set(Level.ship, "Sprite", { texture = "sprites/ship.png" })
    audio.stopMusic()
end

function Level.update(entity, dt)
    local position = ecs.get(Level.ship, "Transform").position
    local direction = (input.isPressed("right") and 1 or 0) - (input.isPressed("left") and 1 or 0)

    Level.time = Level.time + dt
    if input.isJustPressed("left") or input.isJustPressed("right") then
        audio.play("sounds/move.wav", 0.5)
    end
    ecs.set(Level.ship, "Transform", { position = { x = position.x + direction * SPEED * dt, y = position.y } })
    if position.x >= GOAL then
        local best = storage.load("best")

        if not best or Level.time < best.time then
            storage.save("best", { time = math.floor(Level.time * 100) / 100 })
        end
        game.loadScene("scenes/title.json")
    end
end

return Level
```

The ship reaches the right side, the time is saved when it beats the best one, and the title screen comes back with it. The save is `saves/best.json`, next to the folder the game runs from.

## Change it while it runs

Run the game, then change `SPEED` in `level.lua` and save the file: the level starts again with the new speed. Save `controls.lua` with another key for `start`: the scene running starts again, since its script requires the module, and the title writes the new key. A mistake in a script is logged with its file and line, and the script runs again once the file is fixed and saved.

A pause is one more action: `game.setTimeScale(0)` freezes every system, the movements and the animations, and `game.setTimeScale(1)` starts them again. The scripts still run, with a step of 0 seconds, so the one that paused reads the key that resumes.

/*
** EPITECH PROJECT, 2026
** j-climb/src/Game
** File description:
** Game source file
*/

#include "Game.hpp"

#include <utility>

#include "Controls.hpp"
#include "younity/audio/SoundBuffer.hpp"
#include "younity/render/Renderer.hpp"
#include "younity/render/Texture.hpp"

namespace jclimb {

namespace {

constexpr float STEP = 1.0F / 60.0F;
// 16 tiles of 8 pixels, shown 4 times larger
constexpr unsigned int RESOLUTION = 128;
constexpr unsigned int WINDOW_SIZE = RESOLUTION * 4;

// The textures are loaded before the world, which builds the room and draws it from its first step
younity::assets::ResourceManager createResources(const std::filesystem::path &assets)
{
    younity::assets::ResourceManager resources(assets);

    resources.setLoader<younity::render::Texture>(younity::render::Texture::loadFromFile);
    resources.setLoader<younity::audio::SoundBuffer>(younity::audio::SoundBuffer::loadFromFile);
    return resources;
}

} // namespace

Game::Game(const std::filesystem::path &assets)
    : younity::Application(STEP),
      _window(WINDOW_SIZE, WINDOW_SIZE, "J-Climb"),
      _resources(createResources(assets)),
      _world(this->_resources, this->createHost(assets))
{
    this->_window.setLogicalResolution(RESOLUTION, RESOLUTION);
}

void Game::onInput(void)
{
    this->_window.pollEvents();
    if (!this->_window.isOpen()) {
        this->stop();
    }
}

void Game::onFixedUpdate(float step)
{
    this->_input.update();
    this->_world.update(step * this->_timeScale);
}

void Game::onRender(float /* alpha */)
{
    this->_window.clear();
    younity::render::drawEntities(this->_world.getRegistry(), this->_window);
    this->_window.display();
}

younity::scripting::Host Game::createHost(const std::filesystem::path &assets)
{
    younity::scripting::Host host{getActions(this->_input)};

    host.audio.play = [this](const std::string &path, float volume) {
        if (const auto sound = this->_resources.load<younity::audio::SoundBuffer>(path)) {
            this->_sounds.play(*sound, volume);
        }
    };
    host.audio.playMusic = [this](const std::string &path, bool looping) {
        if (path != this->_musicPath) {
            auto music = younity::audio::Music::openFromFile(this->_resources.getRoot() / path);

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
    host.game.quit = [this] { this->stop(); };
    host.game.setTimeScale = [this](float scale) { this->_timeScale = scale; };
    host.saves = assets.parent_path() / "saves";
    return host;
}

} // namespace jclimb

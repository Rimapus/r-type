/*
** EPITECH PROJECT, 2026
** j-climb/src/Game
** File description:
** Game header file
*/

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "World.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/audio/Music.hpp"
#include "younity/audio/SoundPlayer.hpp"
#include "younity/core/Application.hpp"
#include "younity/input/InputManager.hpp"
#include "younity/render/Window.hpp"
#include "younity/scripting/Host.hpp"

namespace jclimb {

/**
 * @brief The window of the game, drawn at 128x128 and shown 4 times larger, and the world it shows. The scripts bind
 *        the controls, play the sounds and the music, quit the game and pause it through the host the game gives them.
 */
class Game : public younity::Application
{
public:
    /**
     * @brief Open the window and load the world.
     *
     * @param assets The assets folder, with the scripts, the sprites and the scene file; the saves go in the "saves"
     *               folder next to it
     *
     * @throw std::runtime_error if the scene file cannot be loaded.
     */
    explicit Game(const std::filesystem::path &assets);

protected:
    /**
     * @brief Read the events of the window, and stop once it is closed.
     */
    void onInput(void) override;

    /**
     * @brief Read the controls and run one step of the world, at the time scale the scripts set.
     *
     * @param step The duration of the step, in seconds
     */
    void onFixedUpdate(float step) override;

    /**
     * @brief Draw the entities of the world.
     *
     * @param alpha Unused: the scripts move the entities by whole pixels
     */
    void onRender(float alpha) override;

private:
    /**
     * @brief Give the scripts the input, the audio, the game loop and the saves.
     *
     * @param assets The assets folder
     *
     * @return The host of the scripts.
     */
    younity::scripting::Host createHost(const std::filesystem::path &assets);

    younity::render::Window _window;
    younity::assets::ResourceManager _resources;
    younity::input::InputManager _input;
    younity::audio::SoundPlayer _sounds;
    // The music playing, and its path in the assets: the same path again goes on from where it was paused
    std::optional<younity::audio::Music> _music;
    std::string _musicPath;
    float _musicVolume = 1.0F;
    // 0 while the scripts pause the game, 1 otherwise
    float _timeScale = 1.0F;
    World _world;
};

} // namespace jclimb

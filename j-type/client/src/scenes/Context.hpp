/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Context
** File description:
** Context header file
*/

#pragma once

#include "audio/Audio.hpp"
#include "input/Controls.hpp"
#include "render/Assets.hpp"
#include "settings/Settings.hpp"
#include "system/Process.hpp"
#include "younity/ecs/SceneManager.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/render/Window.hpp"

namespace jtype {

/**
 * @brief What the scenes of the client share. The client owns it, so it outlives every scene.
 */
struct Context
{
    // Changed by a scene to show the next one
    younity::ecs::SceneManager &scenes;
    // The window, whose colors follow the colorblind setting
    younity::render::Window &window;
    Assets &assets;
    Audio &audio;
    // Read once per step by the client, before the scene
    Controls &controls;
    Settings &settings;
    // The server PLAY joins, from the command line
    const younity::network::Endpoint &server;
    // Set to close the client after this step
    bool &quit;
    // Set by the client at each step while the editor pauses the game: the screens stand still, and the game screen
    // keeps its connection to the server, which goes on without the client
    const bool &paused;
    // J-Climb, the easter egg of the main menu: started once at a time
    Process &jclimb;
};

/**
 * @brief Correct the colors of the window for the color blindness of the settings, or draw them as they are.
 *        Without shaders, the colors stay as they are and the player is told on the error output.
 *
 * @param context What the scenes share
 */
void applyColorblind(Context &context);

} // namespace jtype

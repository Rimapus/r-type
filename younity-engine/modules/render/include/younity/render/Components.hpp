/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/Components
** File description:
** Components header file
*/

#pragma once

#include "younity/assets/ResourceManager.hpp"
#include "younity/ecs/ComponentRegistry.hpp"

namespace younity::render {

/**
 * @brief Register the components of the render module, Sprite, Text, Animation, ParticleEmitter and Particle,
 *        with their fields. The texture of a Sprite and the font of a Text are fields holding their path in the
 *        asset folder: reading one gives the path the resource was loaded from, empty when it has none or did not
 *        come from the resource manager, and writing one loads the resource, or removes it for an empty path.
 *
 * @param components The component registry of the game
 * @param resources The resource manager loading the textures and the fonts, with a loader for both; it must
 *                  outlive the component registry
 *
 * @throw std::invalid_argument if one of these components is already registered.
 */
void registerComponents(ecs::ComponentRegistry &components, assets::ResourceManager &resources);

} // namespace younity::render

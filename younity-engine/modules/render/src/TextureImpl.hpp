/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/TextureImpl
** File description:
** TextureImpl header file, private to the render module
*/

#pragma once

#include "younity/render/Texture.hpp"
#include <SFML/Graphics/Texture.hpp>

namespace younity::render {

/**
 * @brief The SFML texture behind a Texture. In src/: the users of the module never see it.
 */
struct Texture::Impl
{
    sf::Texture texture;
};

} // namespace younity::render

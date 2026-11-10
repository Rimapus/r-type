/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/FontImpl
** File description:
** FontImpl header file, private to the render module
*/

#pragma once

#include "younity/render/Font.hpp"
#include <SFML/Graphics/Font.hpp>

namespace younity::render {

/**
 * @brief The SFML font behind a Font. In src/: the users of the module never see it.
 */
struct Font::Impl
{
    sf::Font font;
};

} // namespace younity::render

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/PixelPerfect
** File description:
** PixelPerfect header file
*/

#pragma once

#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"

namespace younity::render {

/**
 * @brief Get where a game drawn at a logical resolution is shown in the window: scaled by the largest whole
 *        factor that fits, so each logical pixel is a square of screen pixels, and centered between black bars.
 *        A window smaller than the logical resolution shrinks it to fit instead, keeping its proportions:
 *        some rows and columns of pixels are then not shown.
 *
 * @param logicalSize The size the game is drawn at, in logical pixels
 * @param windowSize The size of the window, in pixels
 *
 * @return The area of the window showing the game, in pixels.
 *
 * @throw std::invalid_argument if a width or a height is 0.
 */
[[nodiscard]] Rectf getPixelPerfectArea(Vector2<unsigned int> logicalSize, Vector2<unsigned int> windowSize);

/**
 * @brief Convert a point of the window to the logical resolution, e.g. the mouse cursor to the world of the entities.
 *
 * @param pixel The point, in pixels of the window
 * @param area Where the game is shown in the window, from getPixelPerfectArea()
 * @param logicalSize The size the game is drawn at, in logical pixels
 *
 * @return The point, in logical pixels. A point on a black bar is outside of the game.
 */
[[nodiscard]] Vector2f mapPixelToLogical(Vector2f pixel, const Rectf &area, Vector2<unsigned int> logicalSize);

/**
 * @brief Convert a point of the logical resolution to the window, the opposite of mapPixelToLogical().
 *
 * @param logical The point, in logical pixels
 * @param area Where the game is shown in the window, from getPixelPerfectArea()
 * @param logicalSize The size the game is drawn at, in logical pixels
 *
 * @return The point, in pixels of the window.
 */
[[nodiscard]] Vector2f mapLogicalToPixel(Vector2f logical, const Rectf &area, Vector2<unsigned int> logicalSize);

} // namespace younity::render

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/Window
** File description:
** Window header file
*/

#pragma once

#include <array>
#include <memory>
#include <optional>
#include <string>

#include "younity/core/Clock.hpp"
#include "younity/core/Rect.hpp"
#include "younity/core/Vector2.hpp"

namespace sf {
class RenderTarget;
class RenderTexture;
class RenderWindow;
class Shader;
} // namespace sf

namespace younity::ecs {
struct Transform;
}

namespace younity::render {

struct Sprite;
struct Text;

/**
 * @brief A matrix that changes the colors of the game, written as 3 rows of 3 numbers: each row makes a new channel,
 *        red, then green, then blue, from the red, the green and the blue of the pixel, each from 0 to 1. The identity
 *        {1, 0, 0, 0, 1, 0, 0, 0, 1} keeps the colors.
 */
using ColorMatrix = std::array<float, 9>;

/**
 * @brief Game window, backed by SFML. It also runs the debug interface (Dear ImGui) drawn over the game:
 *        its windows, like drawDebugWindow(), are added between clear() and display().
 */
class Window
{
public:
    /**
     * @brief Open the window and start its debug interface.
     *
     * @param width The width of the window, in pixels
     * @param height The height of the window, in pixels
     * @param title The title of the window
     *
     * @throw std::runtime_error if the debug interface cannot create its font texture.
     */
    Window(unsigned int width, unsigned int height, const std::string &title);
    ~Window(void);

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    /**
     * @brief Draw the game at a fixed logical resolution, e.g. 128x128: the sprites and the texts are drawn in
     *        an image of this size, shown in the window with sharp pixels, see getPixelPerfectArea().
     *        The positions of the entities are then in logical pixels. The debug interface stays at the size of the window.
     *        Call it before the first frame: what is drawn since the last clear() is dropped.
     *
     * @param width The logical width, in pixels
     * @param height The logical height, in pixels
     *
     * @throw std::invalid_argument if the width or the height is 0.
     * @throw std::runtime_error if the image cannot be created, e.g. bigger than the graphics card allows.
     */
    void setLogicalResolution(unsigned int width, unsigned int height);

    /**
     * @brief Change the colors of the game with a matrix applied to each of its pixels by a shader, e.g. to correct
     *        them for a color blind player. The alpha and the debug interface keep their colors.
     *
     * @param matrix The matrix, or nothing to draw the colors as they are
     *
     * @return false if the graphics card has no shaders: the colors are then drawn as they are.
     */
    bool setColorMatrix(const std::optional<ColorMatrix> &matrix);

    /**
     * @brief Show the game in an area of the window instead of the whole window, e.g. the preview of an editor drawn
     *        around it. The game keeps its proportions in the area, centered between black bars, and its sharp pixels
     *        at a logical resolution. mapPixelToWorld() and mapWorldToPixel() follow the area. The debug interface
     *        stays at the size of the window.
     *
     * @param area The area, in pixels from the top left corner of the window, or nothing to show the game in the whole
     *        window, stretched without a logical resolution
     */
    void setGameArea(const std::optional<Rectf> &area);

    /**
     * @brief Check if the window is still open.
     *
     * @return false once the window has been closed.
     */
    [[nodiscard]] bool isOpen(void) const;

    /**
     * @brief Handle the pending window events, e.g. closing the window, and give them to the debug interface.
     */
    void pollEvents(void);

    /**
     * @brief Clear the window before drawing a new frame, start the frame of the debug interface,
     *        and publish the frame time and the frame rate in the metrics store.
     *
     * The metrics are "frame_time", a Timer in seconds, and "fps", a Value. They are recorded
     * on the first clear() of a frame; a second clear() before display() keeps the same frame.
     */
    void clear(void);

    /**
     * @brief Draw a sprite, placed, rotated and scaled by a transform. Does nothing if it has no texture.
     *
     * @param sprite The image to draw
     * @param transform Where to draw it
     */
    void draw(const Sprite &sprite, const ecs::Transform &transform);

    /**
     * @brief Draw a text, placed, rotated and scaled by a transform. Does nothing if it has no font.
     *
     * @param text The text to draw
     * @param transform Where to draw it
     */
    void draw(const Text &text, const ecs::Transform &transform);

    /**
     * @brief Convert a point of the window, e.g. the mouse cursor, to the world where the entities are placed.
     *        They differ once the window is resized: the world keeps its size and is stretched to the window.
     *        At a logical resolution, the world is the logical image: see mapPixelToLogical().
     *
     * @param pixel The point, in pixels from the top left corner of the window
     *
     * @return The point in the world, in pixels.
     */
    [[nodiscard]] Vector2f mapPixelToWorld(Vector2i pixel) const;

    /**
     * @brief Convert a point of the world to the window, e.g. to draw over an entity in the debug interface.
     *
     * @param world The point in the world, in pixels
     *
     * @return The point, in pixels from the top left corner of the window.
     */
    [[nodiscard]] Vector2i mapWorldToPixel(Vector2f world) const;

    /**
     * @brief Display on screen what has been drawn since the last clear, the debug interface over it.
     */
    void display(void);

private:
    /**
     * @brief Get where the game is drawn: the image of the logical resolution if there is one, the window otherwise.
     *
     * @return The target of the sprites and the texts.
     */
    [[nodiscard]] sf::RenderTarget &getGameTarget(void);

    /**
     * @brief Get where the image of the logical resolution is shown in the window, see getPixelPerfectArea().
     *
     * @return The area in pixels of the window, or std::nullopt while the window has no size, e.g. minimized.
     */
    [[nodiscard]] std::optional<Rectf> getCanvasArea(void) const;

    /**
     * @brief Without a logical resolution, set the view of the window so the world is drawn in the area of the game,
     *        see setGameArea().
     */
    void applyGameArea(void);

    /**
     * @brief Show the image of the logical resolution in the window, scaled by getPixelPerfectArea().
     *        The view of the window is set to its pixels.
     */
    void drawCanvas(void);

    /**
     * @brief Without a logical resolution, draw the game in an image of the size of the window while a color matrix
     *        is set, with the view of the window, so the matrix is applied when the image is shown.
     */
    void prepareFilter(void);

    /**
     * @brief Show the image of the game drawn by prepareFilter() in the window, pixel for pixel, through the color
     *        matrix. The view of the window is kept.
     */
    void drawFiltered(void);

    std::unique_ptr<sf::RenderWindow> _window;
    // The image the game is drawn in at its logical resolution, null to draw in the window
    std::unique_ptr<sf::RenderTexture> _canvas;
    // The shader of the color matrix, null to keep the colors
    std::unique_ptr<sf::Shader> _colorShader;
    // The image the game is drawn in before the color matrix when it has no logical resolution, null otherwise
    std::unique_ptr<sf::RenderTexture> _filtered;
    // The area of the window the game is shown in, empty for the whole window
    std::optional<Rectf> _gameArea;
    // Time between two frames of the debug interface
    Clock _debugClock;
    // Time between two frames of the game, published as metrics by clear()
    Clock _frameClock;
    // Set by clear() and reset by display(): the debug interface has one frame open at most
    bool _debugFrameStarted = false;
};

} // namespace younity::render

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/Window
** File description:
** Window source file
*/

#include "younity/render/Window.hpp"
#include "TextConversion.hpp"
#include "TextureImpl.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/render/PixelPerfect.hpp"
#include "younity/render/components/Sprite.hpp"
#include "younity/tools/Global.hpp"
#include "younity/tools/Metric.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Cursor.hpp>
#include <algorithm>
#include <cmath>
#include <imgui-SFML.h>
#include <imgui.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

namespace younity::render {

namespace {

// Applies the color matrix to each pixel, its alpha kept. GLSL reads the rows of the matrix as its columns, so the pixel
// is multiplied on the left to apply the rows as written
constexpr const char *COLOR_MATRIX_SHADER = R"(
uniform sampler2D texture;
uniform mat3 matrix;

void main()
{
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    gl_FragColor = vec4(clamp(pixel.rgb * matrix, 0.0, 1.0), pixel.a) * gl_Color;
}
)";

/**
 * @brief Get the largest rectangle of the proportions of a size that fits in an area, centered in it.
 *
 * @param size The size whose proportions are kept
 * @param area The area to fit in
 *
 * @return The rectangle, in the coordinates of the area.
 */
Rectf fitInArea(sf::Vector2f size, const Rectf &area)
{
    const float scale = std::min(area.width / size.x, area.height / size.y);
    const float width = size.x * scale;
    const float height = size.y * scale;

    return Rectf{area.left + (area.width - width) / 2.0F, area.top + (area.height - height) / 2.0F, width, height};
}

} // namespace

Window::Window(unsigned int width, unsigned int height, const std::string &title)
    : _window(std::make_unique<sf::RenderWindow>(sf::VideoMode({width, height}), title))
{
    if (!ImGui::SFML::Init(*this->_window)) {
        throw std::runtime_error("younity::render::Window: the debug interface cannot create its font texture");
    }
    ImGui::GetIO().IniFilename = nullptr;
}

Window::~Window(void)
{
    // ImGui-SFML frees its cursors, while SFML puts the last one back when the window closes
    const std::optional<sf::Cursor> cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);

    if (cursor) {
        this->_window->setMouseCursor(*cursor);
    }
    ImGui::SFML::Shutdown(*this->_window);
    this->_window.reset();
}

void Window::setLogicalResolution(unsigned int width, unsigned int height)
{
    if (width == 0 || height == 0) {
        throw std::invalid_argument("younity::render::Window: the logical resolution is 0");
    }
    auto canvas = std::make_unique<sf::RenderTexture>();

    if (!canvas->resize({width, height})) {
        throw std::runtime_error("younity::render::Window: cannot create the image of the logical resolution");
    }
    this->_canvas = std::move(canvas);
}

sf::RenderTarget &Window::getGameTarget(void)
{
    if (this->_canvas) {
        return *this->_canvas;
    }
    if (this->_filtered) {
        return *this->_filtered;
    }
    return *this->_window;
}

bool Window::isOpen(void) const
{
    return this->_window->isOpen();
}

void Window::pollEvents(void)
{
    std::optional event = this->_window->pollEvent();

    while (event) {
        ImGui::SFML::ProcessEvent(*this->_window, *event);
        if (event->is<sf::Event::Closed>()) {
            this->_window->close();
        }
        event = this->_window->pollEvent();
    }
}

void Window::clear(void)
{
    // Again at each frame: the area is a part of the window, which may have been resized
    this->applyGameArea();
    this->prepareFilter();
    this->getGameTarget().clear();
    if (!this->_debugFrameStarted) {
        const float frameTime = this->_frameClock.restart();

        ImGui::SFML::Update(*this->_window, sf::seconds(this->_debugClock.restart()));
        this->_debugFrameStarted = true;
        if (frameTime > 0.0F) {
            tools::metrics().record("frame_time", frameTime, tools::MetricKind::Timer);
            tools::metrics().record("fps", 1.0 / frameTime);
        }
    }
}

void Window::draw(const Sprite &sprite, const ecs::Transform &transform)
{
    const Texture::Impl *image = sprite.texture != nullptr ? sprite.texture->getImpl() : nullptr;

    // No texture, or a texture moved from: nothing to draw
    if (image == nullptr) {
        return;
    }

    sf::Sprite drawn(image->texture);

    if (sprite.textureRect) {
        const Recti &rect = *sprite.textureRect;

        drawn.setTextureRect(sf::IntRect({rect.left, rect.top}, {rect.width, rect.height}));
    }
    drawn.setOrigin({sprite.origin.x, sprite.origin.y});
    drawn.setColor(sf::Color(sprite.color.r, sprite.color.g, sprite.color.b, sprite.color.a));
    drawn.setPosition({transform.position.x, transform.position.y});
    drawn.setRotation(sf::degrees(transform.rotation));
    drawn.setScale({transform.scale.x, transform.scale.y});
    this->getGameTarget().draw(drawn);
}

void Window::draw(const Text &text, const ecs::Transform &transform)
{
    const Font::Impl *font = text.font != nullptr ? text.font->getImpl() : nullptr;

    if (font == nullptr) {
        return;
    }

    sf::Text drawn = toSfmlText(text, *font);

    drawn.setPosition({transform.position.x, transform.position.y});
    drawn.setRotation(sf::degrees(transform.rotation));
    drawn.setScale({transform.scale.x, transform.scale.y});
    this->getGameTarget().draw(drawn);
}

std::optional<Rectf> Window::getCanvasArea(void) const
{
    const sf::Vector2u logical = this->_canvas->getSize();
    const sf::Vector2u size = this->_window->getSize();
    const Rectf bounds = this->_gameArea.value_or(Rectf{0.0F, 0.0F, static_cast<float>(size.x), static_cast<float>(size.y)});
    // getPixelPerfectArea() works on whole pixels
    const auto width = static_cast<unsigned int>(std::max(bounds.width, 0.0F));
    const auto height = static_cast<unsigned int>(std::max(bounds.height, 0.0F));

    // A minimized window can have no size: there is nowhere to show the game
    if (size.x == 0 || size.y == 0 || width == 0 || height == 0) {
        return std::nullopt;
    }

    const Rectf area = getPixelPerfectArea({logical.x, logical.y}, {width, height});

    return Rectf{bounds.left + area.left, bounds.top + area.top, area.width, area.height};
}

void Window::setGameArea(const std::optional<Rectf> &area)
{
    this->_gameArea = area;
    this->applyGameArea();
}

void Window::applyGameArea(void)
{
    const sf::Vector2u size = this->_window->getSize();
    // The world keeps the size the window opened with
    sf::View view = this->_window->getDefaultView();

    // At a logical resolution, drawCanvas() places the image itself; a minimized window shows nothing
    if (this->_canvas || size.x == 0 || size.y == 0) {
        return;
    }
    if (this->_gameArea && this->_gameArea->width > 0.0F && this->_gameArea->height > 0.0F) {
        const Rectf area = fitInArea(view.getSize(), *this->_gameArea);
        const auto width = static_cast<float>(size.x);
        const auto height = static_cast<float>(size.y);

        view.setViewport(sf::FloatRect({area.left / width, area.top / height}, {area.width / width, area.height / height}));
    }
    this->_window->setView(view);
}

Vector2f Window::mapPixelToWorld(Vector2i pixel) const
{
    if (this->_canvas) {
        const std::optional<Rectf> area = this->getCanvasArea();
        const sf::Vector2u logical = this->_canvas->getSize();
        const Vector2f point{static_cast<float>(pixel.x), static_cast<float>(pixel.y)};

        return area ? mapPixelToLogical(point, *area, {logical.x, logical.y}) : point;
    }
    const sf::Vector2f world = this->_window->mapPixelToCoords({pixel.x, pixel.y});

    return Vector2f{world.x, world.y};
}

Vector2i Window::mapWorldToPixel(Vector2f world) const
{
    if (this->_canvas) {
        const std::optional<Rectf> area = this->getCanvasArea();
        const sf::Vector2u logical = this->_canvas->getSize();
        const Vector2f pixel = area ? mapLogicalToPixel(world, *area, {logical.x, logical.y}) : world;

        return Vector2i{static_cast<int>(std::floor(pixel.x)), static_cast<int>(std::floor(pixel.y))};
    }
    const sf::Vector2i pixel = this->_window->mapCoordsToPixel({world.x, world.y});

    return Vector2i{pixel.x, pixel.y};
}

void Window::drawCanvas(void)
{
    const sf::Vector2u logical = this->_canvas->getSize();
    const sf::Vector2u size = this->_window->getSize();
    const std::optional<Rectf> area = this->getCanvasArea();

    this->_canvas->display();
    if (!area) {
        return;
    }
    sf::Sprite game(this->_canvas->getTexture());

    // One unit is one screen pixel, also after a resize: the default view would stretch the game
    this->_window->setView(sf::View(sf::FloatRect({0.0F, 0.0F}, {static_cast<float>(size.x), static_cast<float>(size.y)})));
    this->_window->clear();
    game.setPosition({area->left, area->top});
    game.setScale({area->width / static_cast<float>(logical.x), area->height / static_cast<float>(logical.y)});
    this->_window->draw(game, sf::RenderStates(this->_colorShader.get()));
}

bool Window::setColorMatrix(const std::optional<ColorMatrix> &matrix)
{
    if (!matrix) {
        this->_colorShader.reset();
        this->_filtered.reset();
        return true;
    }
    if (!this->_colorShader) {
        auto shader = std::make_unique<sf::Shader>();

        if (!sf::Shader::isAvailable() || !shader->loadFromMemory(COLOR_MATRIX_SHADER, sf::Shader::Type::Fragment)) {
            return false;
        }
        shader->setUniform("texture", sf::Shader::CurrentTexture);
        this->_colorShader = std::move(shader);
    }
    this->_colorShader->setUniform("matrix", sf::Glsl::Mat3(matrix->data()));
    return true;
}

void Window::prepareFilter(void)
{
    const sf::Vector2u size = this->_window->getSize();

    // With a logical resolution the matrix is applied when its image is shown, and a minimized window shows nothing
    if (!this->_colorShader || this->_canvas || size.x == 0 || size.y == 0) {
        this->_filtered.reset();
        return;
    }
    if (!this->_filtered || this->_filtered->getSize() != size) {
        auto filtered = std::make_unique<sf::RenderTexture>();

        // Too big for the graphics card: the game is drawn in the window, without the matrix
        if (!filtered->resize(size)) {
            this->_filtered.reset();
            return;
        }
        this->_filtered = std::move(filtered);
    }
    // The view of the window, so the game is drawn where it would be without the matrix
    this->_filtered->setView(this->_window->getView());
}

void Window::drawFiltered(void)
{
    const sf::Vector2u size = this->_window->getSize();
    const sf::View view = this->_window->getView();

    this->_filtered->display();
    sf::Sprite game(this->_filtered->getTexture());

    // One unit is one pixel of the window while the image is shown, then mapPixelToWorld() finds its view again
    this->_window->setView(sf::View(sf::FloatRect({0.0F, 0.0F}, {static_cast<float>(size.x), static_cast<float>(size.y)})));
    this->_window->clear();
    this->_window->draw(game, sf::RenderStates(this->_colorShader.get()));
    this->_window->setView(view);
}

void Window::display(void)
{
    if (this->_canvas) {
        this->drawCanvas();
    } else if (this->_filtered) {
        this->drawFiltered();
    }
    if (this->_debugFrameStarted) {
        ImGui::SFML::Render(*this->_window);
        this->_debugFrameStarted = false;
    }
    this->_window->display();
}

} // namespace younity::render

/*
** EPITECH PROJECT, 2026
** j-climb/tests/TestGame
** File description:
** TestGame header file
*/

#pragma once

#include <cmath>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "World.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/render/Texture.hpp"
#include "younity/render/components/Sprite.hpp"
#include "younity/scripting/Host.hpp"
#include "younity/scripting/components/Script.hpp"

namespace jclimb::tests {

inline constexpr float STEP = 1.0F / 60.0F;

// Where the P of the first room of rooms.lua is, in pixels
inline const younity::Vector2f FIRST_SPAWN{16.0F, 112.0F};

// The order of the characters in font.png, as in assets/scripts/text.lua
inline constexpr std::string_view FONT_ORDER = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:/!.-,+>";
inline constexpr int GLYPH_WIDTH = 4;
inline constexpr int GLYPH_HEIGHT = 6;

/**
 * @brief The game of the real assets folder, without a window: the test presses the keys, steps the world, and reads
 *        where the robot is and the texts on the screen.
 */
class TestGame
{
public:
    /**
     * @brief Load a scene, the one of the game by default.
     *
     * @param scene The scene file, relative to the assets folder, or absolute for a test room
     */
    explicit TestGame(const std::filesystem::path &scene = SCENE_FILE)
        : _resources(createResources()),
          _world(this->_resources, this->createHost(), scene)
    {
    }

    /**
     * @brief Run steps of the world; an action pressed is only just pressed at the first one.
     */
    void step(int steps)
    {
        for (int count = 0; count < steps; ++count) {
            this->_world.update(STEP);
            this->_pressed.clear();
        }
    }

    /**
     * @brief Press an action and hold it down.
     */
    void press(const std::string &action)
    {
        this->_pressed.insert(action);
        this->_held.insert(action);
    }

    /**
     * @brief Release an action held down.
     */
    void release(const std::string &action)
    {
        this->_held.erase(action);
    }

    /**
     * @brief Press an action for one step, then release it.
     */
    void tap(const std::string &action)
    {
        this->press(action);
        this->step(1);
        this->release(action);
    }

    /**
     * @brief Choose START on the title screen, and wait for the robot in the first room.
     */
    void start(void)
    {
        this->step(2);
        this->tap("jump");
        this->step(3);
    }

    /**
     * @brief Find the first entity running a script, e.g. "scripts/spring.lua".
     */
    [[nodiscard]] std::optional<younity::ecs::Entity> findEntity(std::string_view script) const
    {
        const younity::ecs::Registry &registry = this->_world.getRegistry();

        for (const younity::ecs::Entity entity : registry.getEntitiesWith<younity::scripting::Script>()) {
            if (registry.getComponent<younity::scripting::Script>(entity).path == script) {
                return entity;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Get where the robot is: the entity running player.lua.
     */
    [[nodiscard]] std::optional<younity::Vector2f> findPlayer(void) const
    {
        const std::optional<younity::ecs::Entity> player = this->findEntity("scripts/player.lua");

        if (!player) {
            return std::nullopt;
        }
        return this->_world.getRegistry().getComponent<younity::ecs::Transform>(*player).position;
    }

    /**
     * @brief Get the sprite of the first entity running a script.
     */
    [[nodiscard]] const younity::render::Sprite &getSprite(std::string_view script) const
    {
        return this->_world.getRegistry().getComponent<younity::render::Sprite>(this->findEntity(script).value());
    }

    /**
     * @brief Tell if a line of text on the screen contains a text, e.g. "> START" for the chosen entry of a menu.
     */
    [[nodiscard]] bool shows(std::string_view text) const
    {
        for (const std::string &line : this->readLines()) {
            if (line.contains(text)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Tell if the game asked to quit.
     */
    [[nodiscard]] bool hasQuit(void) const noexcept
    {
        return this->_quit;
    }

private:
    static younity::assets::ResourceManager createResources(void)
    {
        younity::assets::ResourceManager resources(JCLIMB_ASSETS_DIR);

        resources.setLoader<younity::render::Texture>(younity::render::Texture::loadFromFile);
        return resources;
    }

    younity::scripting::Host createHost(void)
    {
        younity::scripting::Host host{
            younity::scripting::Actions{
                                        .isPressed = [this](const std::string &action) { return this->_held.contains(action); },
                                        .isJustPressed = [this](const std::string &action) { return this->_pressed.contains(action); },
                                        .isJustReleased = [](const std::string & /* action */) { return false; },
                                        }
        };

        host.game.quit = [this] { this->_quit = true; };
        return host;
    }

    // The characters of font.png on the screen, by line from the top then from the left; a gap is a space
    [[nodiscard]] std::vector<std::string> readLines(void) const
    {
        const younity::ecs::Registry &registry = this->_world.getRegistry();
        std::map<int, std::map<int, std::pair<char, int>>> glyphs;
        std::vector<std::string> lines;

        for (const younity::ecs::Entity entity :
             registry.getEntitiesWith<younity::render::Sprite, younity::ecs::Transform>()) {
            const auto &rect = registry.getComponent<younity::render::Sprite>(entity).textureRect;
            const younity::ecs::Transform &transform = registry.getComponent<younity::ecs::Transform>(entity);

            if (rect && rect->width == GLYPH_WIDTH && rect->height == GLYPH_HEIGHT) {
                const int width = GLYPH_WIDTH * static_cast<int>(transform.scale.x);

                glyphs[static_cast<int>(std::lround(transform.position.y))]
                      [static_cast<int>(std::lround(transform.position.x))] = {
                          FONT_ORDER.at(static_cast<std::size_t>(rect->left / GLYPH_WIDTH)), width};
            }
        }
        for (const auto &[y, line] : glyphs) {
            std::string text;
            int end = line.begin()->first;

            for (const auto &[x, glyph] : line) {
                if (x > end) {
                    text += ' ';
                }
                text += glyph.first;
                end = x + glyph.second;
            }
            lines.push_back(text);
        }
        return lines;
    }

    std::set<std::string> _held;
    std::set<std::string> _pressed;
    bool _quit = false;
    younity::assets::ResourceManager _resources;
    World _world;
};

} // namespace jclimb::tests

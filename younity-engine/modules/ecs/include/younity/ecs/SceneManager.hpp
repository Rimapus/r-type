/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/SceneManager
** File description:
** SceneManager header file
*/

#pragma once

#include <concepts>
#include <memory>
#include <utility>

#include "younity/ecs/Scene.hpp"

namespace younity::ecs {

/**
 * @brief Owns the active scene of the game, updates it and replaces it by another one.
 */
class SceneManager
{
public:
    /**
     * @brief Create a scene that replaces the active one at the beginning of the next update.
     *        The active scene is only destroyed then: it can ask for the change during its own update.
     *        If it is called several times before an update, the last scene is kept.
     *
     * @tparam SceneType The type of the scene, deriving from Scene
     * @param args The arguments given to the constructor of the scene
     */
    template <std::derived_from<Scene> SceneType, typename... Args>
    void changeScene(Args &&...args)
    {
        this->_nextScene = std::make_unique<SceneType>(std::forward<Args>(args)...);
    }

    /**
     * @brief Apply the change of scene asked since the last update, then update the active scene.
     *        Does nothing if there is no scene yet.
     *
     * @param dt The time elapsed since the last frame, in seconds (delta time)
     */
    void update(float dt);

    /**
     * @brief Check if a scene is active, which is the case from the first update after changeScene().
     *
     * @return true if there is an active scene.
     */
    [[nodiscard]] bool hasActiveScene(void) const noexcept;

    /**
     * @brief Get the active scene.
     *
     * @return The scene updated by update().
     *
     * @throw std::logic_error if there is no active scene.
     */
    [[nodiscard]] Scene &getActiveScene(void);

private:
    std::unique_ptr<Scene> _activeScene;
    std::unique_ptr<Scene> _nextScene;
};

} // namespace younity::ecs

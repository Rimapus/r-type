/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/src/SceneManager
** File description:
** SceneManager source file
*/

#include "younity/ecs/SceneManager.hpp"
#include <stdexcept>

namespace younity::ecs {

void SceneManager::update(float dt)
{
    if (this->_nextScene != nullptr) {
        this->_activeScene = std::move(this->_nextScene);
    }
    if (this->_activeScene != nullptr) {
        this->_activeScene->update(dt);
    }
}

bool SceneManager::hasActiveScene(void) const noexcept
{
    return this->_activeScene != nullptr;
}

Scene &SceneManager::getActiveScene(void)
{
    if (this->_activeScene == nullptr) {
        throw std::logic_error("younity::ecs::SceneManager: there is no active scene");
    }
    return *this->_activeScene;
}

} // namespace younity::ecs

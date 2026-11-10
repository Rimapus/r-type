/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/System
** File description:
** System header file
*/

#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "younity/ecs/Entity.hpp"

namespace younity::ecs {

class Registry;

/**
 * @brief Base of the systems: game logic run once per frame on the entities having all the components it needs.
 */
class System
{
public:
    virtual ~System(void) = default;

    /**
     * @brief Get the name of the system, used by the profiler and the metrics overlay.
     *
     * @return The name set with setName(), or the class name given by Registry::addSystem().
     */
    [[nodiscard]] const std::string &getName(void) const noexcept
    {
        return this->_name;
    }

    /**
     * @brief Rename the system, e.g. to give two systems of the same class different names.
     *
     * @param name The new name, not empty
     *
     * @throw std::invalid_argument if the name is empty.
     */
    void setName(std::string name)
    {
        if (name.empty()) {
            throw std::invalid_argument("younity::ecs::System: the name of a system cannot be empty");
        }
        this->_name = std::move(name);
    }

    /**
     * @brief Run the system on its entities.
     *
     * @param registry The registry owning the entities
     * @param entities The entities having all the components the system needs, sorted by id.
     *                 It is a copy: the registry can change while iterating over it.
     * @param dt The time elapsed since the last frame, in seconds (delta time)
     */
    virtual void update(Registry &registry, const std::vector<Entity> &entities, float dt) = 0;

private:
    std::string _name;
};

} // namespace younity::ecs

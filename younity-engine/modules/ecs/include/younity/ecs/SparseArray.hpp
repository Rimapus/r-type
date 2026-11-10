/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/SparseArray
** File description:
** SparseArray header file
*/

#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "younity/ecs/Entity.hpp"

namespace younity::ecs {

/**
 * @brief Base of all the sparse arrays, whatever their component type.
 */
class ISparseArray
{
public:
    virtual ~ISparseArray(void) = default;

    /**
     * @brief Erase the component of the entity, does nothing if it has none.
     *
     * @param entity The entity to erase the component of
     */
    virtual void erase(Entity entity) = 0;
};

/**
 * @brief Components of one type, each stored at the id of its entity.
 *        The slot of an entity without this component is empty.
 *
 * @tparam Component The type of the stored components
 */
template <typename Component>
class SparseArray : public ISparseArray
{
public:
    /**
     * @brief Insert the component of the entity, replacing the one it already has.
     *
     * @param entity The entity owning the component
     * @param component The component to insert
     *
     * @return The inserted component.
     */
    Component &insert(Entity entity, Component component)
    {
        const std::size_t id = entity.getId();

        if (id >= this->_components.size()) {
            this->_components.resize(id + 1);
        }
        this->_components[id] = std::move(component);
        return *this->_components[id];
    }

    /**
     * @brief Erase the component of the entity, does nothing if it has none.
     *
     * @param entity The entity to erase the component of
     */
    void erase(Entity entity) override
    {
        if (this->has(entity)) {
            this->_components[entity.getId()].reset();
        }
    }

    /**
     * @brief Check if the entity has a component in this array.
     *
     * @param entity The entity to check
     *
     * @return true if the slot of the entity holds a component.
     */
    [[nodiscard]] bool has(Entity entity) const noexcept
    {
        return entity.getId() < this->_components.size() && this->_components[entity.getId()].has_value();
    }

    /**
     * @brief Get the component of the entity.
     *
     * @param entity The entity owning the component
     *
     * @return The component of the entity.
     *
     * @throw std::out_of_range if the entity has no component in this array.
     */
    [[nodiscard]] Component &get(Entity entity)
    {
        if (!this->has(entity)) {
            throw std::out_of_range("younity::ecs::SparseArray: the entity has no such component");
        }
        return *this->_components[entity.getId()];
    }

    /**
     * @brief Get the component of the entity.
     *
     * @param entity The entity owning the component
     *
     * @return The component of the entity.
     *
     * @throw std::out_of_range if the entity has no component in this array.
     */
    [[nodiscard]] const Component &get(Entity entity) const
    {
        if (!this->has(entity)) {
            throw std::out_of_range("younity::ecs::SparseArray: the entity has no such component");
        }
        return *this->_components[entity.getId()];
    }

private:
    std::vector<std::optional<Component>> _components;
};

} // namespace younity::ecs

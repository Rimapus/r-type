/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/Entity
** File description:
** Entity header file
*/

#pragma once

#include <compare>
#include <cstdint>

namespace younity::ecs {

/**
 * @brief Id of an entity.
 */
class Entity
{
public:
    using Id = std::uint32_t;

    /**
     * @brief Create an entity from its id.
     *
     * @param id The id of the entity.
     */
    explicit constexpr Entity(Id id) noexcept
        : _id(id) {}

    /**
     * @brief Get the id of the entity.
     *
     * @return The id, used as the index of its components.
     */
    [[nodiscard]] constexpr Id getId(void) const noexcept
    {
        return this->_id;
    }

    /**
     * @brief Check if two entities have the same id.
     */
    constexpr bool operator==(const Entity &other) const noexcept = default;

    /**
     * @brief Compare two entities by id, so they can be sorted, e.g. in a std::set.
     */
    constexpr auto operator<=>(const Entity &other) const noexcept = default;

private:
    Id _id;
};

} // namespace younity::ecs

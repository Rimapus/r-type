/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/Rect
** File description:
** Rect header file
*/

#pragma once

#include "younity/core/Concepts.hpp"
#include "younity/core/Vector2.hpp"

namespace younity {

/**
 * @brief Axis-aligned rectangle: a position (left, top) and a size (width, height).
 *
 * @tparam T The type of the coordinates
 */
template <Numeric T>
struct Rect
{
    T left{};
    T top{};
    T width{};
    T height{};

    /**
     * @brief Check if a point is inside the rectangle.
     *
     * @param point The point to check
     *
     * @return true if the point is inside the rectangle.
     */
    [[nodiscard]] bool contains(const Vector2<T> &point) const
    {
        return point.x >= this->left &&
               point.x < this->left + this->width &&
               point.y >= this->top &&
               point.y < this->top + this->height;
    }

    /**
     * @brief Check if two rectangles overlap.
     *
     * @param other The other rectangle
     *
     * @return true if the rectangles overlap.
     */
    [[nodiscard]] bool intersects(const Rect &other) const
    {
        return this->left < other.left + other.width &&
               other.left < this->left + this->width &&
               this->top < other.top + other.height &&
               other.top < this->top + this->height;
    }

    /**
     * @brief Check if two rectangles have the same position and size.
     *
     * @param other The rectangle to compare with
     *
     * @return true if the position and the size are equal.
     */
    bool operator==(const Rect &other) const = default;
};

using Rectf = Rect<float>;
using Recti = Rect<int>;

} // namespace younity

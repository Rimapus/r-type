/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/Vector2
** File description:
** Vector2 header file
*/

#pragma once

#include <cmath>

#include "younity/core/Concepts.hpp"

namespace younity {

/**
 * @brief 2D vector, e.g. a position, a velocity or a size.
 *
 * @tparam T The type of the coordinates
 */
template <Numeric T>
struct Vector2
{
    T x{};
    T y{};

    /**
     * @brief Get the length of the vector.
     *
     * @return The length of the vector.
     */
    [[nodiscard]] T length(void) const
    {
        return static_cast<T>(std::sqrt(this->x * this->x + this->y * this->y));
    }

    /**
     * @brief Get the vector with the same direction and a length of 1.
     *
     * @return The normalized vector.
     */
    [[nodiscard]] Vector2 normalized(void) const
    {
        const T currentLength = this->length();

        // Check if currentLength == "the init value of T" (to prevent division by 0)
        if (currentLength == T{}) {
            return *this;
        }
        return *this / currentLength;
    }

    /**
     * @brief Add two vectors.
     *
     * @param other The vector to add
     *
     * @return The sum of the two vectors.
     */
    Vector2 operator+(const Vector2 &other) const
    {
        return Vector2{this->x + other.x, this->y + other.y};
    }

    /**
     * @brief Subtract two vectors.
     *
     * @param other The vector to subtract
     *
     * @return The difference of the two vectors.
     */
    Vector2 operator-(const Vector2 &other) const
    {
        return Vector2{this->x - other.x, this->y - other.y};
    }

    /**
     * @brief Multiply both coordinates by a number.
     *
     * @param scalar The number to multiply by
     *
     * @return The multiplied vector.
     */
    Vector2 operator*(T scalar) const
    {
        return Vector2{this->x * scalar, this->y * scalar};
    }

    /**
     * @brief Divide both coordinates by a number.
     *
     * @param scalar The number to divide by
     *
     * @return The divided vector.
     */
    Vector2 operator/(T scalar) const
    {
        return Vector2{this->x / scalar, this->y / scalar};
    }

    /**
     * @brief Add a vector to this one.
     *
     * @param other The vector to add
     *
     * @return This vector, after the addition.
     */
    Vector2 &operator+=(const Vector2 &other)
    {
        *this = *this + other;
        return *this;
    }

    /**
     * @brief Subtract a vector from this one.
     *
     * @param other The vector to subtract
     *
     * @return This vector, after the subtraction.
     */
    Vector2 &operator-=(const Vector2 &other)
    {
        *this = *this - other;
        return *this;
    }

    /**
     * @brief Multiply both coordinates of this vector by a number.
     *
     * @param scalar The number to multiply by
     *
     * @return This vector, after the multiplication.
     */
    Vector2 &operator*=(T scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    /**
     * @brief Divide both coordinates of this vector by a number.
     *
     * @param scalar The number to divide by, must not be 0
     *
     * @return This vector, after the division.
     */
    Vector2 &operator/=(T scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    /**
     * @brief Check if two vectors have the same coordinates.
     *
     * @param other The vector to compare with
     *
     * @return true if both coordinates are equal.
     */
    bool operator==(const Vector2 &other) const = default;
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;

} // namespace younity

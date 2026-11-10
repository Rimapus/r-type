/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/SparseArray
** File description:
** SparseArray unit tests
*/

#include <criterion/criterion.h>

#include <stdexcept>

#include "younity/ecs/SparseArray.hpp"

using younity::ecs::Entity;
using younity::ecs::SparseArray;

Test(ecs_sparse_array, insert_then_get)
{
    SparseArray<int> array;

    array.insert(Entity(3), 42);
    cr_assert(array.has(Entity(3)));
    cr_assert_eq(array.get(Entity(3)), 42);
}

Test(ecs_sparse_array, insert_replaces_the_component)
{
    SparseArray<int> array;

    array.insert(Entity(0), 1);
    array.insert(Entity(0), 2);
    cr_assert_eq(array.get(Entity(0)), 2);
}

Test(ecs_sparse_array, entity_without_component)
{
    SparseArray<int> array;

    array.insert(Entity(5), 42);
    cr_assert_not(array.has(Entity(2)));
    cr_assert_not(array.has(Entity(10)));
}

Test(ecs_sparse_array, erase_removes_the_component)
{
    SparseArray<int> array;

    array.insert(Entity(0), 42);
    array.erase(Entity(0));
    cr_assert_not(array.has(Entity(0)));
}

Test(ecs_sparse_array, get_a_missing_component_throws)
{
    SparseArray<int> array;

    cr_assert_throw((void)array.get(Entity(0)), std::out_of_range);
}

Test(ecs_sparse_array, const_get_a_missing_component_throws)
{
    SparseArray<int> array;

    array.insert(Entity(0), 42);

    const SparseArray<int> &view = array;

    cr_assert_eq(view.get(Entity(0)), 42);
    cr_assert_throw((void)view.get(Entity(1)), std::out_of_range);
}

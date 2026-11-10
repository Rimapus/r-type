/*
** EPITECH PROJECT, 2026
** younity-engine/tests/assets/AssetError
** File description:
** AssetError unit tests
*/

#include <criterion/criterion.h>

#include <string_view>

#include "younity/assets/AssetError.hpp"

using younity::assets::AssetError;

Test(assets_asset_error, describes_all_errors)
{
    cr_assert_eq(younity::assets::toString(AssetError::NotFound), std::string_view("not found"));
    cr_assert_eq(younity::assets::toString(AssetError::LoadFailed), std::string_view("load failed"));
    cr_assert_eq(younity::assets::toString(AssetError::InvalidPath), std::string_view("invalid path"));
    cr_assert_eq(younity::assets::toString(AssetError::NoLoader), std::string_view("no loader"));
}

Test(assets_asset_error, unknown_value_falls_back)
{
    cr_assert_eq(younity::assets::toString(static_cast<AssetError>(0xFF)), std::string_view("unknown"));
}

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/assets/AssetPath
** File description:
** AssetPath unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>

#include "younity/assets/AssetPath.hpp"

using younity::assets::AssetError;
using younity::assets::normalizeAssetPath;

Test(assets_asset_path, plain_path_is_kept)
{
    cr_assert(*normalizeAssetPath("sprites/ship.png") == std::filesystem::path("sprites/ship.png"));
}

Test(assets_asset_path, ways_to_write_the_same_file_give_one_path)
{
    const std::filesystem::path expected("sprites/ship.png");

    cr_assert(*normalizeAssetPath("./sprites/ship.png") == expected);
    cr_assert(*normalizeAssetPath("sprites//ship.png") == expected);
    cr_assert(*normalizeAssetPath("sprites/enemies/../ship.png") == expected);
    cr_assert(*normalizeAssetPath("sprites/./ship.png") == expected);
}

Test(assets_asset_path, path_going_out_of_the_folder_is_refused)
{
    cr_assert_eq(normalizeAssetPath("../save.dat").error(), AssetError::InvalidPath);
    cr_assert_eq(normalizeAssetPath("sprites/../../save.dat").error(), AssetError::InvalidPath);
}

Test(assets_asset_path, absolute_path_is_refused)
{
    cr_assert_eq(normalizeAssetPath("/etc/passwd").error(), AssetError::InvalidPath);
}

Test(assets_asset_path, empty_path_is_refused)
{
    cr_assert_eq(normalizeAssetPath("").error(), AssetError::InvalidPath);
}

Test(assets_asset_path, folder_is_refused)
{
    cr_assert_eq(normalizeAssetPath(".").error(), AssetError::InvalidPath);
    cr_assert_eq(normalizeAssetPath("sprites/..").error(), AssetError::InvalidPath);
    cr_assert_eq(normalizeAssetPath("sprites/").error(), AssetError::InvalidPath);
}

Test(assets_asset_path, going_back_inside_the_folder_is_kept)
{
    cr_assert(*normalizeAssetPath("sprites/../ship.png") == std::filesystem::path("ship.png"));
    cr_assert(*normalizeAssetPath("a/b/../../ship.png") == std::filesystem::path("ship.png"));
}

Test(assets_asset_path, dots_in_a_name_are_not_a_parent)
{
    cr_assert(*normalizeAssetPath("...") == std::filesystem::path("..."));
    cr_assert(*normalizeAssetPath("..ship.png") == std::filesystem::path("..ship.png"));
}

Test(assets_asset_path, every_way_to_reach_the_parent_is_refused)
{
    cr_assert_eq(normalizeAssetPath("..").error(), AssetError::InvalidPath);
    cr_assert_eq(normalizeAssetPath("a/../..").error(), AssetError::InvalidPath);
    cr_assert_eq(normalizeAssetPath("a/./").error(), AssetError::InvalidPath);
}

#ifdef _WIN32
Test(assets_asset_path, windows_root_is_refused)
{
    cr_assert_eq(normalizeAssetPath("C:ship.png").error(), AssetError::InvalidPath);
    cr_assert_eq(normalizeAssetPath("\\ship.png").error(), AssetError::InvalidPath);
}
#endif

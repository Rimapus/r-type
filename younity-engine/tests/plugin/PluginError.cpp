/*
** EPITECH PROJECT, 2026
** younity-engine/tests/plugin/PluginError
** File description:
** PluginError unit tests
*/

#include <criterion/criterion.h>

#include <string_view>

#include "younity/plugin/PluginError.hpp"

using younity::plugin::PluginErrorCode;

Test(plugin_error, describes_each_code)
{
    cr_assert_eq(younity::plugin::toString(PluginErrorCode::NotFound), std::string_view("not found"));
    cr_assert_eq(younity::plugin::toString(PluginErrorCode::OpenFailed), std::string_view("cannot open"));
    cr_assert_eq(
        younity::plugin::toString(PluginErrorCode::MissingDescriptor), std::string_view("missing descriptor"));
    cr_assert_eq(younity::plugin::toString(PluginErrorCode::AbiMismatch), std::string_view("ABI mismatch"));
    cr_assert_eq(
        younity::plugin::toString(PluginErrorCode::InvalidDescriptor), std::string_view("invalid descriptor"));
    cr_assert_eq(
        younity::plugin::toString(PluginErrorCode::InvalidConfig), std::string_view("invalid configuration"));
}

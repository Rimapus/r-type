/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/Global
** File description:
** Global logger registry unit tests
*/

#include <criterion/criterion.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "younity/logger/Global.hpp"
#include "younity/logger/Level.hpp"

using younity::logger::Level;

namespace {

// Criterion runs the tests in parallel: each test uses its own folder, removed before and after it
std::filesystem::path testFolder(const char *test)
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / ("younity_global_" + std::string(test));

    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    return folder;
}

std::string readFile(const std::filesystem::path &path)
{
    const std::ifstream file(path);
    std::ostringstream content;

    content << file.rdbuf();
    return content.str();
}

} // namespace

// Criterion runs each test in its own process, so the global registry starts clean in every test
Test(logger_global, registry_is_always_the_same)
{
    cr_assert_eq(&younity::logger::registry(), &younity::logger::registry());
}

Test(logger_global, get_uses_the_global_registry)
{
    cr_assert_eq(&younity::logger::get("ecs"), &younity::logger::registry().get("ecs"));
}

Test(logger_global, default_level_is_info)
{
    cr_assert(younity::logger::get("ecs").getLevel() == Level::Info);
}

Test(logger_global, writes_to_the_console_by_default)
{
    cr_assert(younity::logger::registry().hasSink("console"));
}

Test(logger_global, writes_to_no_file_by_default)
{
    cr_assert_not(younity::logger::registry().hasSink("file"));
}

Test(logger_global, enable_file_writes_the_messages_to_the_file)
{
    const std::filesystem::path path = testFolder("enable_file") / "younity.log";

    younity::logger::enableFile(path);
    younity::logger::get("ecs").error("written to the file");
    cr_assert(younity::logger::registry().hasSink("file"));
    cr_assert(readFile(path).find("written to the file") != std::string::npos);
    younity::logger::disableFile();
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_global, disable_file_stops_writing_to_the_file)
{
    const std::filesystem::path path = testFolder("disable_file") / "younity.log";

    younity::logger::enableFile(path);
    cr_assert(younity::logger::disableFile());
    younity::logger::get("ecs").error("not written");
    cr_assert_not(younity::logger::registry().hasSink("file"));
    cr_assert_eq(readFile(path).find("not written"), std::string::npos);
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_global, disable_file_without_file_does_nothing)
{
    cr_assert_not(younity::logger::disableFile());
}

Test(logger_global, enable_a_file_that_cannot_be_opened_throws)
{
    const std::filesystem::path folder = testFolder("enable_a_file_that_cannot_be_opened");

    cr_assert_throw(younity::logger::enableFile(folder), std::runtime_error);
    cr_assert_not(younity::logger::registry().hasSink("file"));
    std::filesystem::remove_all(folder);
}

Test(logger_global, file_variable_enables_the_file)
{
    const std::filesystem::path path = testFolder("file_variable") / "younity.log";

    setenv("YOUNITY_LOG_FILE", path.c_str(), 1);
    younity::logger::get("ecs").error("written to the file");
    cr_assert(younity::logger::registry().hasSink("file"));
    cr_assert(readFile(path).find("written to the file") != std::string::npos);
    younity::logger::disableFile();
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_global, file_variable_that_cannot_be_opened_is_ignored)
{
    const std::filesystem::path folder = testFolder("file_variable_that_cannot_be_opened");

    setenv("YOUNITY_LOG_FILE", folder.c_str(), 1);
    cr_assert_not(younity::logger::registry().hasSink("file"));
    cr_assert(younity::logger::registry().hasSink("console"));
    std::filesystem::remove_all(folder);
}

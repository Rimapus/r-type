/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/FileSink
** File description:
** FileSink unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <fstream>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>

#include "younity/logger/FileSink.hpp"
#include "younity/logger/Formatter.hpp"
#include "younity/logger/Level.hpp"
#include "younity/logger/Record.hpp"

using younity::logger::FileSink;
using younity::logger::formatRecord;
using younity::logger::Level;
using younity::logger::Record;

namespace {

Record makeRecord(Level level, const char *message)
{
    return Record(level, "ecs", message, std::source_location::current(), Record::Clock::now());
}

// Criterion runs the tests in parallel: each test uses its own folder, removed before and after it
std::filesystem::path testFolder(const char *test)
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / ("younity_file_sink_" + std::string(test));

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

Test(logger_file_sink, writes_the_formatted_line)
{
    const std::filesystem::path path = testFolder("writes_the_formatted_line") / "younity.log";
    const Record record = makeRecord(Level::Info, "message");

    {
        FileSink sink(path);

        sink.write(record);
    }
    cr_assert(readFile(path) == formatRecord(record) + "\n", "got \"%s\"", readFile(path).c_str());
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_file_sink, writes_every_level_without_colors)
{
    const std::filesystem::path path = testFolder("writes_every_level_without_colors") / "younity.log";

    {
        FileSink sink(path);

        sink.write(makeRecord(Level::Debug, "debug"));
        sink.write(makeRecord(Level::Critical, "critical"));
    }

    const std::string content = readFile(path);

    cr_assert(content.find("debug") != std::string::npos);
    cr_assert(content.find("critical") != std::string::npos);
    cr_assert_eq(content.find('\033'), std::string::npos);
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_file_sink, replaces_the_previous_content_by_default)
{
    const std::filesystem::path path = testFolder("replaces_the_previous_content_by_default") / "younity.log";

    std::ofstream(path) << "previous run\n";
    {
        FileSink sink(path);

        sink.write(makeRecord(Level::Info, "new run"));
    }
    cr_assert_eq(readFile(path).find("previous run"), std::string::npos);
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_file_sink, append_keeps_the_previous_content)
{
    const std::filesystem::path path = testFolder("append_keeps_the_previous_content") / "younity.log";

    std::ofstream(path) << "previous run\n";
    {
        FileSink sink(path, true);

        sink.write(makeRecord(Level::Info, "new run"));
    }

    const std::string content = readFile(path);

    cr_assert(content.starts_with("previous run\n"));
    cr_assert(content.find("new run") != std::string::npos);
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_file_sink, creates_the_missing_folders)
{
    const std::filesystem::path folder = testFolder("creates_the_missing_folders");
    const std::filesystem::path path = folder / "logs" / "today" / "younity.log";

    {
        FileSink sink(path);

        sink.write(makeRecord(Level::Info, "message"));
    }
    cr_assert(std::filesystem::exists(path));
    std::filesystem::remove_all(folder);
}

Test(logger_file_sink, errors_are_written_at_once)
{
    const std::filesystem::path path = testFolder("errors_are_written_at_once") / "younity.log";
    FileSink sink(path);

    sink.write(makeRecord(Level::Error, "error"));
    // The sink is still open: the line must already be in the file
    cr_assert(readFile(path).find("error") != std::string::npos);
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_file_sink, keeps_its_path)
{
    const std::filesystem::path path = testFolder("keeps_its_path") / "younity.log";
    const FileSink sink(path);

    cr_assert(sink.getPath() == path);
    std::filesystem::remove_all(path.parent_path());
}

Test(logger_file_sink, cannot_open_throws)
{
    const std::filesystem::path folder = testFolder("cannot_open_throws");

    // A folder cannot be opened as a file
    cr_assert_throw(FileSink sink(folder), std::runtime_error);
    std::filesystem::remove_all(folder);
}

/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/src/Global
** File description:
** Global source file
*/

#include "younity/logger/Global.hpp"
#include "younity/logger/ConsoleSink.hpp"
#include "younity/logger/FileSink.hpp"
#include <cstdlib>
#include <exception>
#include <memory>

namespace younity::logger {

namespace {

constexpr std::string_view CONSOLE_SINK_NAME = "console";
constexpr std::string_view FILE_SINK_NAME = "file";

/**
 * @brief Write to the file given by the YOUNITY_LOG_FILE variable, if it is set.
 *        A file that cannot be opened is reported as an error, without stopping the program.
 *
 * @param globalRegistry The registry being created, not reachable by registry() yet
 */
void enableFileFromEnvironment(LoggerRegistry &globalRegistry)
{
    const char *path = std::getenv("YOUNITY_LOG_FILE");

    if (path == nullptr || path[0] == '\0') {
        return;
    }
    try {
        globalRegistry.addSink(std::string(FILE_SINK_NAME), std::make_unique<FileSink>(path));
    } catch (const std::exception &exception) {
        globalRegistry.get("logger").error("YOUNITY_LOG_FILE is ignored: {}", exception.what());
    }
}

/**
 * @brief Create the global registry, writing to the console so the messages are shown without any setup.
 *
 * @return The new registry.
 */
std::unique_ptr<LoggerRegistry> createGlobalRegistry(void)
{
    auto globalRegistry = std::make_unique<LoggerRegistry>();

    globalRegistry->addSink(std::string(CONSOLE_SINK_NAME), std::make_unique<ConsoleSink>());
    enableFileFromEnvironment(*globalRegistry);
    return globalRegistry;
}

} // namespace

LoggerRegistry &registry(void)
{
    // Created at the first call: the C++ standard makes this initialization thread safe
    static const std::unique_ptr<LoggerRegistry> globalRegistry = createGlobalRegistry();

    return *globalRegistry;
}

Logger &get(std::string_view name)
{
    return registry().get(name);
}

void enableFile(const std::filesystem::path &path, bool append)
{
    registry().addSink(std::string(FILE_SINK_NAME), std::make_unique<FileSink>(path, append));
}

bool disableFile(void)
{
    return registry().removeSink(FILE_SINK_NAME);
}

} // namespace younity::logger

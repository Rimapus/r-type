/*
** EPITECH PROJECT, 2026
** younity-engine/modules/plugin/src/Plugin
** File description:
** Plugin source file
*/

#include "younity/plugin/Plugin.hpp"

#include <cstring>
#include <format>
#include <string>
#include <utility>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace younity::plugin {

namespace {

#ifdef _WIN32
using LibraryHandle = HMODULE;
#else
using LibraryHandle = void *;
#endif

[[nodiscard]] PluginError makeError(PluginErrorCode code, const std::filesystem::path &path, std::string message)
{
    return PluginError{.code = code, .path = path, .message = std::move(message)};
}

[[nodiscard]] LibraryHandle openLibrary(const std::filesystem::path &path, std::string &error)
{
#ifdef _WIN32
    const HMODULE library = ::LoadLibraryW(path.c_str());

    if (library == nullptr) {
        error = std::format("cannot open the library, Windows error {}", static_cast<unsigned long>(::GetLastError()));
    }
    return library;
#else
    // Clear a previous error, then take the one of this call
    ::dlerror();
    void *const library = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);

    if (library == nullptr) {
        const char *const message = ::dlerror();

        error = message != nullptr ? message : "cannot open the library";
    }
    return library;
#endif
}

void closeLibrary(LibraryHandle library) noexcept
{
#ifdef _WIN32
    (void)::FreeLibrary(library);
#else
    (void)::dlclose(library);
#endif
}

[[nodiscard]] PluginDescriptorFunction findDescriptor(LibraryHandle library) noexcept
{
    PluginDescriptorFunction function = nullptr;

#ifdef _WIN32
    const FARPROC symbol = ::GetProcAddress(library, YOUNITY_PLUGIN_DESCRIPTOR_SYMBOL);
#else
    void *const symbol = ::dlsym(library, YOUNITY_PLUGIN_DESCRIPTOR_SYMBOL);
#endif

    // ISO C++ does not allow a cast between an object pointer and a function pointer
    static_assert(sizeof(function) == sizeof(symbol), "a function pointer and a library symbol must have the same size");
    std::memcpy(&function, &symbol, sizeof(function));
    return function;
}

[[nodiscard]] bool hasText(const char *text) noexcept
{
    return text != nullptr && text[0] != '\0';
}

} // namespace

Plugin::~Plugin(void)
{
    this->unload();
}

Plugin::Plugin(Plugin &&other) noexcept
    : _library(std::exchange(other._library, nullptr)), _descriptor(std::exchange(other._descriptor, nullptr)), _instance(std::exchange(other._instance, nullptr)), _path(std::move(other._path)), _name(std::move(other._name)), _version(std::move(other._version)), _started(std::exchange(other._started, false))
{
}

Plugin &Plugin::operator=(Plugin &&other) noexcept
{
    if (this == &other) {
        return *this;
    }
    this->unload();
    this->_library = std::exchange(other._library, nullptr);
    this->_descriptor = std::exchange(other._descriptor, nullptr);
    this->_instance = std::exchange(other._instance, nullptr);
    this->_path = std::move(other._path);
    this->_name = std::move(other._name);
    this->_version = std::move(other._version);
    this->_started = std::exchange(other._started, false);
    return *this;
}

std::expected<Plugin, PluginError> Plugin::loadFromFile(const std::filesystem::path &path)
{
    std::error_code fileError;

    if (path.empty() || !std::filesystem::exists(path, fileError)) {
        if (fileError) {
            return std::unexpected(
                makeError(PluginErrorCode::OpenFailed, path, "cannot check the plugin file: " + fileError.message()));
        }
        return std::unexpected(makeError(PluginErrorCode::NotFound, path, "the plugin file does not exist"));
    }

    std::string openError;
    const LibraryHandle library = openLibrary(path, openError);

    if (library == nullptr) {
        return std::unexpected(makeError(PluginErrorCode::OpenFailed, path, std::move(openError)));
    }
    const PluginDescriptorFunction descriptorFunction = findDescriptor(library);

    if (descriptorFunction == nullptr) {
        closeLibrary(library);
        return std::unexpected(makeError(
            PluginErrorCode::MissingDescriptor, path,
            "the plugin does not export " + std::string(YOUNITY_PLUGIN_DESCRIPTOR_SYMBOL)));
    }
    const PluginDescriptor *const descriptor = descriptorFunction();

    if (descriptor == nullptr) {
        closeLibrary(library);
        return std::unexpected(
            makeError(PluginErrorCode::InvalidDescriptor, path, "the plugin descriptor is null"));
    }
    if (descriptor->abiVersion != YOUNITY_PLUGIN_ABI_VERSION) {
        // The descriptor lives in the library: read it before closing the library
        const std::uint32_t abiVersion = descriptor->abiVersion;

        closeLibrary(library);
        return std::unexpected(makeError(
            PluginErrorCode::AbiMismatch, path,
            std::format("the plugin ABI version is {}, the engine expects {}", abiVersion, YOUNITY_PLUGIN_ABI_VERSION)));
    }
    if (!hasText(descriptor->name)) {
        closeLibrary(library);
        return std::unexpected(
            makeError(PluginErrorCode::InvalidDescriptor, path, "the plugin descriptor has no name"));
    }
    if (!hasText(descriptor->version)) {
        closeLibrary(library);
        return std::unexpected(
            makeError(PluginErrorCode::InvalidDescriptor, path, "the plugin descriptor has no version"));
    }
    if (descriptor->create == nullptr) {
        closeLibrary(library);
        return std::unexpected(
            makeError(PluginErrorCode::InvalidDescriptor, path, "the plugin descriptor has no create callback"));
    }
    if (descriptor->destroy == nullptr) {
        closeLibrary(library);
        return std::unexpected(
            makeError(PluginErrorCode::InvalidDescriptor, path, "the plugin descriptor has no destroy callback"));
    }
    if (descriptor->start == nullptr) {
        closeLibrary(library);
        return std::unexpected(
            makeError(PluginErrorCode::InvalidDescriptor, path, "the plugin descriptor has no start callback"));
    }
    if (descriptor->update == nullptr) {
        closeLibrary(library);
        return std::unexpected(
            makeError(PluginErrorCode::InvalidDescriptor, path, "the plugin descriptor has no update callback"));
    }
    if (descriptor->stop == nullptr) {
        closeLibrary(library);
        return std::unexpected(
            makeError(PluginErrorCode::InvalidDescriptor, path, "the plugin descriptor has no stop callback"));
    }

    // Everything that can throw happens before create(), so an exception never leaks the context
    std::filesystem::path pluginPath;
    std::string name;
    std::string version;

    try {
        pluginPath = path;
        name = descriptor->name;
        version = descriptor->version;
    } catch (...) {
        closeLibrary(library);
        throw;
    }

    void *instance = nullptr;

    try {
        instance = descriptor->create();
    } catch (...) {
        closeLibrary(library);
        throw;
    }
    return Plugin(library, descriptor, instance, std::move(pluginPath), std::move(name), std::move(version));
}

std::string_view Plugin::getName(void) const noexcept
{
    return this->_name;
}

std::string_view Plugin::getVersion(void) const noexcept
{
    return this->_version;
}

const std::filesystem::path &Plugin::getPath(void) const noexcept
{
    return this->_path;
}

bool Plugin::isStarted(void) const noexcept
{
    return this->_started;
}

void Plugin::start(void) noexcept
{
    if (this->_descriptor == nullptr || this->_started) {
        return;
    }
    this->_descriptor->start(this->_instance);
    this->_started = true;
}

void Plugin::update(double deltaTime) noexcept
{
    if (this->_descriptor == nullptr || !this->_started) {
        return;
    }
    this->_descriptor->update(this->_instance, deltaTime);
}

void Plugin::stop(void) noexcept
{
    if (this->_descriptor == nullptr || !this->_started) {
        return;
    }
    this->_descriptor->stop(this->_instance);
    this->_started = false;
}

void Plugin::unload(void) noexcept
{
    this->stop();
    if (this->_descriptor != nullptr) {
        this->_descriptor->destroy(this->_instance);
    }
    this->_instance = nullptr;
    this->_descriptor = nullptr;
    this->_started = false;
    this->_name.clear();
    this->_version.clear();
    this->_path.clear();
    if (this->_library != nullptr) {
        closeLibrary(static_cast<LibraryHandle>(this->_library));
        this->_library = nullptr;
    }
}

Plugin::Plugin(
    void *library,
    const PluginDescriptor *descriptor,
    void *instance,
    std::filesystem::path path,
    std::string name,
    std::string version) noexcept
    : _library(library), _descriptor(descriptor), _instance(instance), _path(std::move(path)), _name(std::move(name)), _version(std::move(version))
{
}

} // namespace younity::plugin

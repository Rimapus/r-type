/*
** EPITECH PROJECT, 2026
** j-type/common/src/Paths
** File description:
** Paths source file
*/

#include "jtype/common/Paths.hpp"

#include <stdexcept>
#include <system_error>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <string>
#include <windows.h>
#elif defined(__APPLE__)
#include <cstdint>
#include <mach-o/dyld.h>
#include <string>
#endif

namespace jtype {

namespace {

/**
 * @brief Ask the system for the path of the running executable.
 *
 * @return The path, empty if the system cannot give it.
 */
std::filesystem::path executablePath(void)
{
#if defined(_WIN32)
    std::wstring path(MAX_PATH, L'\0');

    // The path is truncated when the buffer is too small: retry with a bigger one
    for (;;) {
        const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));

        if (length == 0) {
            return {};
        }
        if (length < path.size()) {
            path.resize(length);
            return path;
        }
        path.resize(path.size() * 2);
    }
#elif defined(__APPLE__)
    std::uint32_t size = 0;

    // The first call only gives the size of the path
    (void)_NSGetExecutablePath(nullptr, &size);
    std::string path(size, '\0');

    if (_NSGetExecutablePath(path.data(), &size) != 0) {
        return {};
    }
    path.resize(path.find('\0'));
    return path;
#else
    std::error_code error;
    std::filesystem::path path = std::filesystem::read_symlink("/proc/self/exe", error);

    return error ? std::filesystem::path() : path;
#endif
}

} // namespace

std::filesystem::path executableDir(void)
{
    const std::filesystem::path path = executablePath();
    std::error_code error;

    if (path.empty()) {
        throw std::runtime_error("cannot find the path of the executable");
    }
    // The path can go through a symbolic link or "..": the folder is the one of the real file
    const std::filesystem::path real = std::filesystem::weakly_canonical(path, error);

    return (error ? path : real).parent_path();
}

} // namespace jtype

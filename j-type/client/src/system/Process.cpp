/*
** EPITECH PROJECT, 2026
** j-type/client/src/system/Process
** File description:
** Process source file
*/

#include "system/Process.hpp"

#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <process.h>
#include <windows.h>
#else
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char **environ;
#endif

namespace jtype {

namespace {

// The name of the program first, then its arguments, then a null pointer, as spawn and exec want them
std::vector<std::string> getCommandLine(const std::filesystem::path &program, const std::vector<std::string> &arguments)
{
    std::vector<std::string> line{program.string()};

    line.insert(line.end(), arguments.begin(), arguments.end());
    return line;
}

std::vector<char *> getPointers(std::vector<std::string> &line)
{
    std::vector<char *> pointers;

    for (std::string &argument : line) {
        pointers.push_back(argument.data());
    }
    pointers.push_back(nullptr);
    return pointers;
}

} // namespace

Process::~Process(void)
{
#ifdef _WIN32
    if (this->_process != 0) {
        CloseHandle(reinterpret_cast<HANDLE>(this->_process));
    }
#endif
}

bool Process::start(const std::filesystem::path &program, const std::vector<std::string> &arguments)
{
    if (this->isRunning()) {
        return false;
    }
    std::vector<std::string> line = getCommandLine(program, arguments);
    std::vector<char *> pointers = getPointers(line);

#ifdef _WIN32
    const std::intptr_t handle = _spawnv(_P_NOWAIT, line.front().c_str(), pointers.data());

    if (handle == -1) {
        return false;
    }
    this->_process = handle;
#else
    pid_t pid = 0;

    if (posix_spawn(&pid, line.front().c_str(), nullptr, nullptr, pointers.data(), environ) != 0) {
        return false;
    }
    this->_process = pid;
#endif
    return true;
}

bool Process::isRunning(void)
{
    if (this->_process == 0) {
        return false;
    }
#ifdef _WIN32
    if (WaitForSingleObject(reinterpret_cast<HANDLE>(this->_process), 0) == WAIT_TIMEOUT) {
        return true;
    }
    CloseHandle(reinterpret_cast<HANDLE>(this->_process));
#else
    // Reaps the program once it ended, so it does not stay as a zombie
    int status = 0;

    if (waitpid(static_cast<pid_t>(this->_process), &status, WNOHANG) == 0) {
        return true;
    }
#endif
    this->_process = 0;
    return false;
}

} // namespace jtype

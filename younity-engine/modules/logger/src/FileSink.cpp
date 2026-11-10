/*
** EPITECH PROJECT, 2026
** younity-engine/modules/logger/src/FileSink
** File description:
** FileSink source file
*/

#include "younity/logger/FileSink.hpp"
#include "younity/logger/Formatter.hpp"
#include <iostream>
#include <stdexcept>
#include <system_error>

namespace younity::logger {

FileSink::FileSink(const std::filesystem::path &path, bool append)
    : _path(path)
{
    std::error_code error;

    // An error is ignored here: the folder is then missing, and the opening below fails
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }
    this->_file.open(path, append ? std::ios::app : std::ios::trunc);
    if (!this->_file.is_open()) {
        throw std::runtime_error("younity::logger::FileSink: cannot open \"" + path.string() + "\"");
    }
}

void FileSink::write(const Record &record)
{
    this->_file << formatRecord(record) << '\n';
    if (record.getLevel() >= Level::Error) {
        this->_file.flush();
    }
    // Reported once, and directly on the error output: a sink must not log, it would wait for its own mutex
    if (this->_file.fail() && !this->_failed) {
        this->_failed = true;
        std::cerr << "younity::logger::FileSink: cannot write to \"" << this->_path.string() << "\"" << std::endl;
    }
}

const std::filesystem::path &FileSink::getPath(void) const noexcept
{
    return this->_path;
}

} // namespace younity::logger

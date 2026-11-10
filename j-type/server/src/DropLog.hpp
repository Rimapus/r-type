/*
** EPITECH PROJECT, 2026
** j-type/server/src/DropLog
** File description:
** DropLog header file
*/

#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

namespace jtype {

/**
 * @brief Count the datagrams the server rejects and report them at most once per interval, so a flood of garbage
 *        cannot flood the log as well.
 */
class DropLog
{
public:
    /**
     * @brief Create a log reporting at most once per interval.
     *
     * @param interval The shortest delay between two reports
     */
    explicit DropLog(std::chrono::milliseconds interval = std::chrono::seconds(1)) noexcept;

    /**
     * @brief Record a rejected datagram.
     *
     * @param reason Why it was rejected
     * @param now The current time
     *
     * @return The line to log when the interval has passed since the last report, empty otherwise.
     */
    [[nodiscard]] std::string reject(std::string_view reason, std::chrono::steady_clock::time_point now);

private:
    std::chrono::milliseconds _interval;
    std::chrono::steady_clock::time_point _lastReport{};
    // The reason of the first rejection of the interval, the others are only counted
    std::string _firstReason;
    std::size_t _count = 0;
};

} // namespace jtype

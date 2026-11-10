/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/ThreadSafeQueue
** File description:
** ThreadSafeQueue header file
*/

#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

#include "younity/network/Limits.hpp"

namespace younity::network {

/**
 * @brief First-in first-out queue safe to use from several threads.
 *
 * One thread pushes while another one pops: the game loop polls with tryPop
 * while the network threads push or wait. Closing the queue wakes every
 * waiting pop with an empty result, so the threads can stop without waiting
 * forever. The queue holds at most its capacity, so a stalled consumer does
 * not let a producer reserve memory without limit. Uses the same mutex
 * pattern as younity::logger::SinkDispatcher.
 *
 * @tparam Item The type of the queued values, e.g. ReceivedDatagram
 */
template <typename Item>
class ThreadSafeQueue
{
public:
    /**
     * @brief Create an empty open queue with the default capacity.
     */
    ThreadSafeQueue(void) = default;

    /**
     * @brief Create an empty open queue with a custom capacity.
     *
     * @param capacity The largest number of values kept, 0 refuses every push
     */
    explicit ThreadSafeQueue(std::size_t capacity) noexcept
        : _capacity(capacity)
    {
    }

    ThreadSafeQueue(const ThreadSafeQueue &) = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

    /**
     * @brief Add a value at the back of the queue.
     *
     * @param value The value to queue, moved from
     *
     * @return false if the queue is closed or full and the value was dropped.
     */
    bool push(Item value)
    {
        std::lock_guard<std::mutex> lock(this->_mutex);
        if (this->_closed || this->_items.size() >= this->_capacity) {
            return false;
        }
        this->_items.push_back(std::move(value));
        this->_ready.notify_one();
        return true;
    }

    /**
     * @brief Get the largest number of values the queue keeps.
     *
     * @return The capacity given at construction.
     */
    [[nodiscard]] std::size_t getCapacity(void) const noexcept
    {
        return this->_capacity;
    }

    /**
     * @brief Take the oldest value without waiting.
     *
     * @return The value, or nothing if the queue is empty.
     */
    [[nodiscard]] std::optional<Item> tryPop(void)
    {
        std::lock_guard<std::mutex> lock(this->_mutex);
        if (this->_items.empty()) {
            return std::nullopt;
        }
        std::optional<Item> value(std::move(this->_items.front()));

        this->_items.pop_front();
        return value;
    }

    /**
     * @brief Take the oldest value, waiting until one arrives or the queue closes.
     *
     * @return The value, or nothing if the queue was closed while empty.
     */
    [[nodiscard]] std::optional<Item> waitPop(void)
    {
        std::unique_lock<std::mutex> lock(this->_mutex);

        this->_ready.wait(lock, [this] {
            return this->_closed || !this->_items.empty();
        });
        if (this->_items.empty()) {
            return std::nullopt;
        }
        std::optional<Item> value(std::move(this->_items.front()));

        this->_items.pop_front();
        return value;
    }

    /**
     * @brief Wake every waiting pop with an empty result and refuse new pushes.
     *        Queued values stay readable until reset.
     */
    void close(void) noexcept
    {
        std::lock_guard<std::mutex> lock(this->_mutex);

        this->_closed = true;
        this->_ready.notify_all();
    }

    /**
     * @brief Drop every queued value and reopen the queue after a close.
     */
    void reset(void) noexcept
    {
        std::lock_guard<std::mutex> lock(this->_mutex);

        this->_items.clear();
        this->_closed = false;
        this->_ready.notify_all();
    }

    /**
     * @brief Get the number of queued values.
     *
     * @return The number of values waiting to be popped.
     */
    [[nodiscard]] std::size_t getSize(void) const
    {
        std::lock_guard<std::mutex> lock(this->_mutex);

        return this->_items.size();
    }

    /**
     * @brief Check if no value is queued.
     *
     * @return true if nothing waits to be popped.
     */
    [[nodiscard]] bool isEmpty(void) const
    {
        std::lock_guard<std::mutex> lock(this->_mutex);

        return this->_items.empty();
    }

    /**
     * @brief Check if the queue refuses new pushes.
     *
     * @return true once close was called and reset was not.
     */
    [[nodiscard]] bool isClosed(void) const
    {
        std::lock_guard<std::mutex> lock(this->_mutex);

        return this->_closed;
    }

private:
    mutable std::mutex _mutex;
    std::condition_variable _ready;
    std::deque<Item> _items;
    std::size_t _capacity = DEFAULT_QUEUE_CAPACITY;
    bool _closed = false;
};

} // namespace younity::network

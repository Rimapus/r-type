/*
** EPITECH PROJECT, 2026
** younity-engine/tests/network/Queue
** File description:
** ThreadSafeQueue unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <cstddef>
#include <optional>
#include <thread>
#include <vector>

#include "younity/network/Datagram.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/ThreadSafeQueue.hpp"

using younity::network::Endpoint;
using younity::network::ReceivedDatagram;
using younity::network::ThreadSafeQueue;

Test(network_queue, push_and_try_pop_are_fifo)
{
    ThreadSafeQueue<int> queue;

    cr_assert(queue.push(1));
    cr_assert(queue.push(2));
    cr_assert(queue.push(3));
    cr_assert_eq(*queue.tryPop(), 1);
    cr_assert_eq(*queue.tryPop(), 2);
    cr_assert_eq(*queue.tryPop(), 3);
    cr_assert_eq(queue.tryPop(), std::nullopt);
    cr_assert(queue.isEmpty());
}

Test(network_queue, try_pop_empty_returns_nullopt)
{
    ThreadSafeQueue<int> queue;

    cr_assert(queue.isEmpty());
    cr_assert_eq(queue.getSize(), std::size_t{0});
    cr_assert_eq(queue.tryPop(), std::nullopt);
}

Test(network_queue, wait_pop_returns_pushed)
{
    ThreadSafeQueue<int> queue;

    cr_assert(queue.push(42));
    cr_assert_eq(*queue.waitPop(), 42);
    cr_assert(queue.isEmpty());
}

Test(network_queue, wait_pop_wakes_on_close)
{
    ThreadSafeQueue<int> queue;
    std::optional<int> woken;

    std::thread waiter([&queue, &woken] {
        woken = queue.waitPop();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.close();
    waiter.join();
    cr_assert_eq(woken, std::nullopt);
    cr_assert(queue.isClosed());
}

Test(network_queue, push_after_close_returns_false)
{
    ThreadSafeQueue<int> queue;

    queue.close();
    cr_assert_not(queue.push(1));
    cr_assert_eq(queue.tryPop(), std::nullopt);
}

Test(network_queue, reset_reopens)
{
    ThreadSafeQueue<int> queue;

    queue.close();
    cr_assert_not(queue.push(1));
    queue.reset();
    cr_assert_not(queue.isClosed());
    cr_assert(queue.push(7));
    cr_assert_eq(*queue.tryPop(), 7);
}

Test(network_queue, reset_drops_queued_values)
{
    ThreadSafeQueue<int> queue;

    cr_assert(queue.push(1));
    cr_assert(queue.push(2));
    queue.reset();
    cr_assert(queue.isEmpty());
    cr_assert_eq(queue.tryPop(), std::nullopt);
}

Test(network_queue, size_tracks)
{
    ThreadSafeQueue<int> queue;

    cr_assert(queue.push(1));
    cr_assert(queue.push(2));
    cr_assert_eq(queue.getSize(), std::size_t{2});
    cr_assert(queue.tryPop().has_value());
    cr_assert_eq(queue.getSize(), std::size_t{1});
}

Test(network_queue, carries_datagrams)
{
    ThreadSafeQueue<ReceivedDatagram> queue;
    ReceivedDatagram sent;

    sent.from = Endpoint::loopback(40001);
    sent.bytes = {std::byte{'h'}, std::byte{'i'}};
    cr_assert(queue.push(std::move(sent)));

    const std::optional<ReceivedDatagram> received = queue.tryPop();

    cr_assert(received.has_value());
    cr_assert_eq(received->from, Endpoint::loopback(40001));
    cr_assert_eq(received->bytes, (std::vector<std::byte>{std::byte{'h'}, std::byte{'i'}}));
}

Test(network_queue, concurrent_producers_keep_every_value)
{
    ThreadSafeQueue<int> queue;

    std::thread first([&queue] {
        for (int value = 0; value < 100; value += 1) {
            cr_assert(queue.push(value));
        }
    });
    std::thread second([&queue] {
        for (int value = 100; value < 200; value += 1) {
            cr_assert(queue.push(value));
        }
    });

    first.join();
    second.join();
    cr_assert_eq(queue.getSize(), std::size_t{200});

    std::vector<bool> seen(200, false);

    for (std::size_t count = 0; count < 200; count += 1) {
        const std::optional<int> value = queue.tryPop();

        cr_assert(value.has_value());
        cr_assert_eq(seen.at(static_cast<std::size_t>(*value)), false);
        seen.at(static_cast<std::size_t>(*value)) = true;
    }
    cr_assert(queue.isEmpty());
}

Test(network_queue, default_capacity_is_bounded)
{
    ThreadSafeQueue<int> queue;

    cr_assert_eq(queue.getCapacity(), younity::network::DEFAULT_QUEUE_CAPACITY);
    for (std::size_t value = 0; value < queue.getCapacity(); value += 1) {
        cr_assert(queue.push(static_cast<int>(value)));
    }
    cr_assert_not(queue.push(-1));
    cr_assert_eq(queue.getSize(), queue.getCapacity());
    cr_assert_eq(*queue.tryPop(), 0);
}

Test(network_queue, full_queue_keeps_the_oldest_values)
{
    ThreadSafeQueue<int> queue(2);

    cr_assert(queue.push(1));
    cr_assert(queue.push(2));
    cr_assert_not(queue.push(3));
    cr_assert_eq(queue.getSize(), std::size_t{2});
    cr_assert_eq(*queue.tryPop(), 1);
    cr_assert_eq(*queue.tryPop(), 2);
    cr_assert_eq(queue.tryPop(), std::nullopt);
}

Test(network_queue, zero_capacity_refuses_every_push)
{
    ThreadSafeQueue<int> queue(0);

    cr_assert_eq(queue.getCapacity(), std::size_t{0});
    cr_assert_not(queue.push(1));
    cr_assert(queue.isEmpty());
}

Test(network_queue, reset_reopens_a_full_queue)
{
    ThreadSafeQueue<int> queue(1);

    cr_assert(queue.push(1));
    cr_assert_not(queue.push(2));
    queue.close();
    queue.reset();
    cr_assert(queue.push(3));
    cr_assert_eq(*queue.tryPop(), 3);
}

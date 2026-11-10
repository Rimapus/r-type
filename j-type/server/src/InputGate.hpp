/*
** EPITECH PROJECT, 2026
** j-type/server/src/InputGate
** File description:
** InputGate header file
*/

#pragma once

#include <cstdint>
#include <optional>

namespace jtype {

/**
 * @brief Keep only the inputs of a client that are newer than the last one, so a duplicated or reordered datagram
 *        never brings back keys the client already released.
 */
class InputGate
{
public:
    /**
     * @brief Check an input against the last one accepted, and remember it when it is newer.
     *
     * @param tick The step the client read the keys at
     *
     * @return true for the first input, and for every input newer than the last one accepted.
     */
    [[nodiscard]] bool accept(std::uint32_t tick) noexcept;

private:
    // The last step accepted, empty until the first input
    std::optional<std::uint32_t> _tick;
};

} // namespace jtype

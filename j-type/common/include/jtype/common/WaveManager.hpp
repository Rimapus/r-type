/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/WaveManager
** File description:
** WaveManager header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include "jtype/common/Waves.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/ecs/Registry.hpp"

namespace jtype {

/**
 * @brief Plays the waves of Bydos one after the other: once the Bydos of a wave are destroyed or gone, the pause of the
 *        next one, then its groups, each Bydo of a group after the delay of the group, just past the right edge where
 *        the formation of the group places it, between the ceiling and the floor. The walls and the zappers of a wave
 *        come with it and scroll away without holding the next one. After the last wave, the waves start again from the
 *        first one, their delays shorter by WAVE_SPEEDUP each time, down to MIN_DELAY_FACTOR. A wave with a boss brings
 *        it with its groups, and once the boss and the Bydos of that wave are gone, the game is won and no wave comes
 *        anymore.
 *        Each player after the first one adds a quarter of the Bydos of each group. The last Bydo of a group marked drop
 *        carries a power-up, chosen at random.
 *        The randomness comes from a seed: the same seed gives the same game.
 */
class WaveManager
{
public:
    /**
     * @brief Create the manager, before the pause of the first wave.
     *
     * @param waves The waves to play, e.g. from loadWaves()
     * @param seed The seed of the random heights
     *
     * @throw std::invalid_argument if there is no wave.
     */
    WaveManager(std::vector<Wave> waves, std::uint32_t seed);

    /**
     * @brief Move the waves forward and spawn the Bydos that are due. The pause of a wave waits until the registry has
     *        no Bydo and no boss left.
     *
     * @param registry The registry the Bydos are created in
     * @param dt The duration of the step, in seconds
     * @param players The players in the game, at least one counts
     */
    void update(younity::ecs::Registry &registry, float dt, std::size_t players);

    /**
     * @brief Get how many waves have started, the ones played again included.
     *
     * @return 0 during the pause of the first wave.
     */
    [[nodiscard]] std::size_t getStartedWaves(void) const noexcept;

    /**
     * @brief Tell if the boss of a wave is beaten: the game is won.
     *
     * @return true once the wave of a boss is over.
     */
    [[nodiscard]] bool isWon(void) const noexcept;

private:
    /**
     * @brief A group of the current wave and the Bydos it still has to bring.
     */
    struct Running
    {
        WaveGroup group;
        // Its Bydos with the ones of the other players
        std::size_t count = 0;
        std::size_t left = 0;
        // Seconds before its next Bydo
        float timer = 0.0F;
    };

    /**
     * @brief Start the next wave: its groups bring their first Bydo at once, and its obstacles and its boss come in.
     *
     * @param registry The registry the boss is created in
     * @param players The players in the game
     */
    void startWave(younity::ecs::Registry &registry, std::size_t players);

    /**
     * @brief Get where the next Bydo of a group appears, from its formation.
     *
     * @param running The group
     *
     * @return Just past the right edge, further for the back of a V, and inside the height of the screen.
     */
    [[nodiscard]] younity::Vector2f getSpawnPosition(const Running &running);

    std::vector<Wave> _waves;
    std::mt19937 _random;
    std::size_t _started = 0;
    float _pause = 0.0F;
    // The delays of the current round of waves are multiplied by it
    float _speed = 1.0F;
    std::vector<Running> _running;
    // The wave started last brought a boss
    bool _hasBoss = false;
    bool _isWon = false;
};

} // namespace jtype

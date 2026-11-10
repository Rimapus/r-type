/*
** EPITECH PROJECT, 2026
** j-type/common/src/WaveManager
** File description:
** WaveManager source file
*/

#include "jtype/common/WaveManager.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Enemy.hpp"

namespace jtype {

namespace {

// The power-ups a carrier can leave, one of them at random
constexpr std::array<PowerUpType, 3> POWER_UPS{PowerUpType::ExtraLife, PowerUpType::Shield, PowerUpType::SpreadShot};

// The heights a Bydo appears between, so its sprite is between the ceiling and the floor
constexpr float SPAWN_TOP = TERRAIN_HEIGHT + BYDO_ORIGIN.y;
constexpr float SPAWN_BOTTOM = WORLD_HEIGHT - TERRAIN_HEIGHT - BYDO_SIZE.y + BYDO_ORIGIN.y;

} // namespace

WaveManager::WaveManager(std::vector<Wave> waves, std::uint32_t seed)
    : _waves(std::move(waves)),
      _random(seed)
{
    if (this->_waves.empty()) {
        throw std::invalid_argument("jtype::WaveManager: no wave to play");
    }
    this->_pause = this->_waves.front().pause;
}

void WaveManager::update(younity::ecs::Registry &registry, float dt, std::size_t players)
{
    if (this->_isWon) {
        return;
    }
    if (this->_running.empty()) {
        // The waves never pile up: the pause starts once the last Bydo of the wave before is destroyed or gone
        if (!registry.getEntitiesWith<Enemy>().empty() || !registry.getEntitiesWith<Boss>().empty()) {
            return;
        }
        // The boss does not leave the screen: gone, it is destroyed
        if (this->_hasBoss) {
            this->_isWon = true;
            return;
        }
        this->_pause -= dt;
        if (this->_pause > 0.0F) {
            return;
        }
        // The time past the end of the pause already belongs to the wave
        dt = -this->_pause;
        this->startWave(registry, players);
    }
    for (Running &running : this->_running) {
        running.timer -= dt;
        while (running.left > 0 && running.timer <= 0.0F) {
            const younity::ecs::Entity bydo =
                spawnBydo(registry, this->getSpawnPosition(running), running.group.type, running.group.pattern);

            if (running.group.drop && running.left == 1) {
                std::uniform_int_distribution<std::size_t> pick(0, POWER_UPS.size() - 1);

                registry.getComponent<Enemy>(bydo).drop = POWER_UPS[pick(this->_random)];
            }
            --running.left;
            running.timer += running.group.delay * this->_speed;
        }
    }
    std::erase_if(this->_running, [](const Running &running) { return running.left == 0; });
    if (this->_running.empty()) {
        this->_pause = this->_waves[this->_started % this->_waves.size()].pause;
    }
}

std::size_t WaveManager::getStartedWaves(void) const noexcept
{
    return this->_started;
}

bool WaveManager::isWon(void) const noexcept
{
    return this->_isWon;
}

void WaveManager::startWave(younity::ecs::Registry &registry, std::size_t players)
{
    const Wave &wave = this->_waves[this->_started % this->_waves.size()];
    const std::size_t extraPlayers = std::max<std::size_t>(players, 1) - 1;

    // A new round of the waves: faster than the last one
    if (this->_started > 0 && this->_started % this->_waves.size() == 0) {
        this->_speed = std::max(this->_speed * WAVE_SPEEDUP, MIN_DELAY_FACTOR);
    }
    ++this->_started;
    for (const WaveGroup &group : wave.groups) {
        const std::size_t count = group.count + (group.count * extraPlayers / 4);

        this->_running.push_back(Running{group, count, count, 0.0F});
    }
    for (const WaveObstacle &obstacle : wave.obstacles) {
        if (obstacle.kind == ObstacleKind::Block) {
            spawnWall(registry, obstacle.top, obstacle.bottom);
        } else {
            spawnZapper(registry, obstacle.top, obstacle.bottom);
        }
    }
    this->_hasBoss = wave.boss.has_value();
    if (wave.boss == BossType::Incubator) {
        spawnIncubator(registry, players);
    }
}

younity::Vector2f WaveManager::getSpawnPosition(const Running &running)
{
    const float x = WORLD_WIDTH + BYDO_ORIGIN.x + BYDO_SPAWN_MARGIN;
    const float center = SPAWN_TOP + (running.group.height * (SPAWN_BOTTOM - SPAWN_TOP));
    const std::size_t index = running.count - running.left;
    younity::Vector2f offset{};

    switch (running.group.formation) {
    case Formation::Random:
        return {x, std::uniform_real_distribution<float>(SPAWN_TOP, SPAWN_BOTTOM)(this->_random)};
    case Formation::Line:
        break;
    case Formation::Column:
        // The first one on top, the column centered on the height
        offset.y = (static_cast<float>(index) - (static_cast<float>(running.count - 1) / 2.0F)) * FORMATION_GAP;
        break;
    case Formation::V: {
        // The first one leads, the next two one step back above and below it, the next two two steps back...
        const auto step = static_cast<float>((index + 1) / 2);
        const float side = index % 2 == 1 ? -1.0F : 1.0F;

        offset = {step * FORMATION_GAP, side * step * FORMATION_GAP};
        break;
    }
    }
    return {x + offset.x, std::clamp(center + offset.y, SPAWN_TOP, SPAWN_BOTTOM)};
}

} // namespace jtype

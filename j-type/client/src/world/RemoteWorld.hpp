/*
** EPITECH PROJECT, 2026
** j-type/client/src/world/RemoteWorld
** File description:
** RemoteWorld header file
*/

#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "jtype/common/EntityType.hpp"
#include "jtype/common/Messages.hpp"
#include "render/Assets.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"

namespace jtype {

/**
 * @brief The copy of the server world drawn here: one local entity per entity of the server,
 *        and over each ship the marker of its player, see layoutPlayerMarker().
 */
class RemoteWorld
{
public:
    /**
     * @brief Create an empty world.
     *
     * @param registry The registry holding the copies, it must outlive the world
     * @param assets The textures and the sprites, they must outlive the world
     * @param reduceFlashes true to draw the explosions smaller and dimmer, see getFlashLook()
     */
    RemoteWorld(younity::ecs::Registry &registry, Assets &assets, bool reduceFlashes = false);

    /**
     * @brief Create, move and remove the entities so they are the ones of the snapshot. A ship with more health than
     *        PLAYER_HEALTH has a shield: a bubble is drawn around it.
     *
     * @param snapshot The world to draw, from SnapshotBuffer::step()
     * @param local The slot of this client, or nothing while it has not joined, to write YOU under its ship
     */
    void apply(const SnapshotMessage &snapshot, std::optional<std::uint8_t> local);

private:
    /**
     * @brief A sprite drawn at a fixed distance from an entity, which follows it.
     */
    struct Attached
    {
        younity::ecs::Entity entity;
        // From the position of the entity, in pixels of the game area
        younity::Vector2f offset;
    };

    /**
     * @brief An entity of the server, its copy drawn here and the sprites that follow it.
     */
    struct RemoteEntity
    {
        younity::ecs::Entity entity;
        EntityType type;
        std::vector<Attached> markers;
        // The bubble around a ship whose shield takes the next hit
        std::optional<younity::ecs::Entity> shield = std::nullopt;
    };

    /**
     * @brief Create an entity drawn with an animated sprite of the sprite file.
     *
     * @param sheet The sprite
     *
     * @return The new entity.
     */
    [[nodiscard]] younity::ecs::Entity create(const SpriteSheet &sheet);

    /**
     * @brief Create the sprites of the marker of a ship, nothing for another type.
     *
     * @param type The type of the entity
     *
     * @return The sprites, not placed yet.
     */
    [[nodiscard]] std::vector<Attached> createMarkers(EntityType type);

    /**
     * @brief Draw the bubble around a ship while it has a shield, and remove it once the shield is gone.
     *
     * @param remote The copy of the entity
     * @param state The entity in the snapshot
     */
    void updateShield(RemoteEntity &remote, const EntityState &state);

    /**
     * @brief Remove the copy of an entity and the sprites that follow it.
     *
     * @param remote The entity
     */
    void destroy(const RemoteEntity &remote);

    younity::ecs::Registry &_registry;
    Assets &_assets;
    bool _reduceFlashes;
    std::unordered_map<std::uint32_t, RemoteEntity> _entities;
    std::optional<std::uint8_t> _local;
};

} // namespace jtype

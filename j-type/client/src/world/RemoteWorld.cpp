/*
** EPITECH PROJECT, 2026
** j-type/client/src/world/RemoteWorld
** File description:
** RemoteWorld source file
*/

#include "world/RemoteWorld.hpp"

#include <memory>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "jtype/common/Gameplay.hpp"
#include "render/BitmapFont.hpp"
#include "render/Flashes.hpp"
#include "render/PlayerMarker.hpp"
#include "render/Screen.hpp"
#include "younity/render/AnimationClip.hpp"
#include "younity/render/Texture.hpp"
#include "younity/render/components/Animation.hpp"
#include "younity/render/components/Sprite.hpp"

namespace jtype {

namespace {

// Over the ships and under the HUD, as the interface of the sprite file
constexpr int LABEL_LAYER = 4;

// The bubble drawn around a shielded ship
constexpr std::string_view SHIELD_SPRITE = "shield_bubble";

} // namespace

RemoteWorld::RemoteWorld(younity::ecs::Registry &registry, Assets &assets, bool reduceFlashes)
    : _registry(registry),
      _assets(assets),
      _reduceFlashes(reduceFlashes)
{
}

void RemoteWorld::apply(const SnapshotMessage &snapshot, std::optional<std::uint8_t> local)
{
    std::unordered_set<std::uint32_t> seen;

    // YOU moves to another ship once the server tells this client its slot
    if (local != this->_local) {
        this->_local = local;
        for (auto &[id, remote] : this->_entities) {
            for (const Attached &marker : remote.markers) {
                this->_registry.destroyEntity(marker.entity);
            }
            remote.markers = this->createMarkers(remote.type);
        }
    }
    for (const EntityState &state : snapshot.entities) {
        auto found = this->_entities.find(state.id);

        // The server reuses the ids of the destroyed entities: another type is another entity
        if (found != this->_entities.end() && found->second.type != state.type) {
            this->destroy(found->second);
            this->_entities.erase(found);
            found = this->_entities.end();
        }
        const FlashLook look = getFlashLook(state.type, this->_reduceFlashes);

        if (found == this->_entities.end()) {
            const RemoteEntity remote{this->create(this->_assets.getSprite(state.type)), state.type,
                                      this->createMarkers(state.type)};

            this->_registry.getComponent<younity::render::Sprite>(remote.entity).color = look.color;
            found = this->_entities.emplace(state.id, remote).first;
        }
        younity::ecs::Transform transform = toScreen(state.x, state.y);

        transform.scale = younity::Vector2f{SCALE * look.scale, SCALE * look.scale};
        this->_registry.addComponent(found->second.entity, transform);
        for (const Attached &marker : found->second.markers) {
            this->_registry.addComponent(marker.entity, toScreen(state.x + marker.offset.x, state.y + marker.offset.y));
        }
        this->updateShield(found->second, state);
        seen.insert(state.id);
    }
    for (auto it = this->_entities.begin(); it != this->_entities.end();) {
        if (seen.contains(it->first)) {
            ++it;
        } else {
            this->destroy(it->second);
            it = this->_entities.erase(it);
        }
    }
}

younity::ecs::Entity RemoteWorld::create(const SpriteSheet &sheet)
{
    younity::render::AnimationClip clip = younity::render::AnimationClip::fromGrid(
        {.frameSize = sheet.frameSize, .columns = sheet.columns, .first = sheet.first, .count = sheet.count},
        sheet.frameDuration, sheet.loop);
    const younity::ecs::Entity entity = this->_registry.createEntity();

    this->_registry.addComponent(entity, younity::render::Sprite{this->_assets.loadTexture(sheet.path), clip.frames.front(),
                                                                 sheet.layer, sheet.origin});
    this->_registry.addComponent(entity, younity::render::Animation{.clip = std::move(clip)});
    return entity;
}

std::vector<RemoteWorld::Attached> RemoteWorld::createMarkers(EntityType type)
{
    const std::optional<PlayerMarker> marker = layoutPlayerMarker(type, this->_local);
    std::vector<Attached> markers;

    if (!marker) {
        return markers;
    }
    markers.push_back(Attached{this->create(this->_assets.getSprite(marker->tag)), marker->tagOffset});
    if (marker->label) {
        const std::shared_ptr<const younity::render::Texture> font = this->_assets.loadTexture(getFontPath(marker->label->color));

        for (const Glyph &glyph : layoutText(marker->label->string)) {
            const younity::ecs::Entity letter = this->_registry.createEntity();

            this->_registry.addComponent(letter, younity::render::Sprite{font, glyph.frame, LABEL_LAYER});
            markers.push_back(Attached{
                letter, {marker->label->position.x + glyph.x, marker->label->position.y}
            });
        }
    }
    return markers;
}

void RemoteWorld::updateShield(RemoteEntity &remote, const EntityState &state)
{
    const bool shielded = getPlayerSlot(state.type).has_value() && state.health > PLAYER_HEALTH;

    if (shielded && !remote.shield) {
        remote.shield = this->create(this->_assets.getSprite(SHIELD_SPRITE));
    } else if (!shielded && remote.shield) {
        this->_registry.destroyEntity(*remote.shield);
        remote.shield.reset();
    }
    if (remote.shield) {
        this->_registry.addComponent(*remote.shield, toScreen(state.x, state.y));
    }
}

void RemoteWorld::destroy(const RemoteEntity &remote)
{
    this->_registry.destroyEntity(remote.entity);
    for (const Attached &marker : remote.markers) {
        this->_registry.destroyEntity(marker.entity);
    }
    if (remote.shield) {
        this->_registry.destroyEntity(*remote.shield);
    }
}

} // namespace jtype

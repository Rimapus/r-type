/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/Components
** File description:
** Components source file
*/

#include "younity/render/Components.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "younity/render/components/Animation.hpp"
#include "younity/render/components/Particle.hpp"
#include "younity/render/components/ParticleEmitter.hpp"
#include "younity/render/components/Sprite.hpp"
#include "younity/render/components/Text.hpp"

namespace younity::render {

namespace {

/**
 * @brief Get the path of a resource, the value of its field.
 *
 * @tparam Resource The type of the resource, Texture or Font
 * @param resources The resource manager that loaded it
 * @param resource The resource of the component, possibly empty
 *
 * @return Its path in the asset folder, or an empty path when it has none or was not loaded by the manager.
 */
template <typename Resource>
std::string getPathOf(const assets::ResourceManager &resources, const std::shared_ptr<const Resource> &resource)
{
    const std::optional<std::filesystem::path> path = resources.findPath(resource);

    return path ? path->generic_string() : std::string();
}

/**
 * @brief Load the resource a field names by its path.
 *
 * @tparam Resource The type of the resource, Texture or Font
 * @param resources The resource manager loading it
 * @param path Its path in the asset folder, or an empty path for no resource
 *
 * @return The resource, or an empty pointer for an empty path.
 *
 * @throw std::invalid_argument if the resource cannot be loaded, so the field keeps its value.
 */
template <typename Resource>
std::shared_ptr<const Resource> loadPath(assets::ResourceManager &resources, const std::string &path)
{
    if (path.empty()) {
        return nullptr;
    }
    const auto resource = resources.load<Resource>(path);

    if (!resource) {
        throw std::invalid_argument("younity::render: cannot load " + path + ": " + std::string(assets::toString(resource.error())));
    }
    return *resource;
}

} // namespace

void registerComponents(ecs::ComponentRegistry &components, assets::ResourceManager &resources)
{
    components.add<Sprite>("Sprite")
        .field<Sprite, std::string>(
            "texture", [&resources](const Sprite &sprite) { return getPathOf(resources, sprite.texture); },
            [&resources](Sprite &sprite, std::string path) { sprite.texture = loadPath<Texture>(resources, path); })
        .field("textureRect", &Sprite::textureRect)
        .field("layer", &Sprite::layer)
        .field("origin", &Sprite::origin)
        .field("visible", &Sprite::visible)
        .field("color", &Sprite::color);
    components.add<Text>("Text")
        .field<Text, std::string>(
            "font", [&resources](const Text &text) { return getPathOf(resources, text.font); },
            [&resources](Text &text, std::string path) { text.font = loadPath<Font>(resources, path); })
        .field("string", &Text::string)
        .field("characterSize", &Text::characterSize)
        .field("color", &Text::color)
        .field("layer", &Text::layer)
        .field("origin", &Text::origin)
        .field("visible", &Text::visible);
    components.add<Animation>("Animation")
        .field("frames", &Animation::clip, &AnimationClip::frames)
        .field("frameDuration", &Animation::clip, &AnimationClip::frameDuration)
        .field("loop", &Animation::clip, &AnimationClip::loop)
        .field("frame", &Animation::frame)
        .field("elapsed", &Animation::elapsed)
        .field("finished", &Animation::finished);
    components.add<ParticleEmitter>("ParticleEmitter")
        .field("rate", &ParticleEmitter::rate)
        .field("burst", &ParticleEmitter::burst)
        .field("emitting", &ParticleEmitter::emitting)
        .field("lifetime", &ParticleEmitter::lifetime)
        .field("speed", &ParticleEmitter::speed)
        .field("speedVariation", &ParticleEmitter::speedVariation)
        .field("direction", &ParticleEmitter::direction)
        .field("spread", &ParticleEmitter::spread);
    components.add<Particle>("Particle").field("velocity", &Particle::velocity).field("remaining", &Particle::remaining);
}

} // namespace younity::render

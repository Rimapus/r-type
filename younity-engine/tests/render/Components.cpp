/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/Components
** File description:
** Render components registration unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "../TestSupport.hpp"
#include "younity/assets/ResourceManager.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/render/Components.hpp"
#include "younity/render/Font.hpp"
#include "younity/render/Texture.hpp"
#include "younity/render/components/Animation.hpp"
#include "younity/render/components/Sprite.hpp"
#include "younity/render/components/Text.hpp"

using younity::Color;
using younity::Recti;
using younity::assets::ResourceManager;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::FieldValue;
using younity::ecs::Registry;
using younity::render::Font;
using younity::render::Sprite;
using younity::render::Text;
using younity::render::Texture;
using younity::test::skipWithoutDisplay;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "render" / "fixtures";

// The textures and the fonts of the fixtures, read by the fields of the components
ResourceManager makeResources(void)
{
    ResourceManager resources(FIXTURES);

    resources.setLoader<Texture>(Texture::loadFromFile);
    resources.setLoader<Font>(Font::loadFromFile);
    return resources;
}

} // namespace

Test(render_components, register_components_with_their_fields)
{
    ComponentRegistry components;
    ResourceManager resources;

    younity::render::registerComponents(components, resources);
    cr_assert(components.getNames() == (std::vector<std::string>{"Animation", "Particle", "ParticleEmitter", "Sprite", "Text"}));
    cr_assert(components.get("Sprite").getFieldNames() ==
              (std::vector<std::string>{"texture", "textureRect", "layer", "origin", "visible", "color"}));
    cr_assert(components.get("Text").getFieldNames() ==
              (std::vector<std::string>{"font", "string", "characterSize", "color", "layer", "origin", "visible"}));
    cr_assert(components.get("Animation").getFieldNames() ==
              (std::vector<std::string>{"frames", "frameDuration", "loop", "frame", "elapsed", "finished"}));
    cr_assert(components.get("ParticleEmitter").getFieldNames() ==
              (std::vector<std::string>{"rate", "burst", "emitting", "lifetime", "speed", "speedVariation", "direction", "spread"}));
    cr_assert(components.get("Particle").getFieldNames() == (std::vector<std::string>{"velocity", "remaining"}));
}

Test(render_components, sprite_is_drawn_as_it_is_by_default)
{
    cr_assert(Sprite{}.color == (Color{255, 255, 255, 255}));
}

Test(render_components, write_the_texture_rect_and_the_color_by_name)
{
    ComponentRegistry components;
    ResourceManager resources;
    Registry registry;
    const Entity entity = registry.createEntity();

    younity::render::registerComponents(components, resources);
    registry.addComponent(entity, Sprite{});
    registry.addComponent(entity, Text{});
    components.get("Sprite").setField(registry, entity, "textureRect", std::optional<Recti>(Recti{0, 0, 16, 16}));
    components.get("Sprite").setField(registry, entity, "color", Color{255, 255, 255, 128});
    components.get("Text").setField(registry, entity, "color", Color{255, 0, 0});
    cr_assert(registry.getComponent<Sprite>(entity).textureRect == (Recti{0, 0, 16, 16}));
    cr_assert(registry.getComponent<Sprite>(entity).color == (Color{255, 255, 255, 128}));
    cr_assert(components.get("Text").getField(registry, entity, "color") == FieldValue(Color{255, 0, 0}));
}

Test(render_components, write_and_read_the_animation_clip_by_name)
{
    ComponentRegistry components;
    ResourceManager resources;
    Registry registry;
    const Entity entity = registry.createEntity();

    younity::render::registerComponents(components, resources);
    registry.addComponent(entity, younity::render::Animation{});
    components.get("Animation").setField(registry, entity, "frames", std::vector<Recti>{
                                                                         Recti{0, 0, 8, 8},
                                                                         Recti{8, 0, 8, 8}
    });
    components.get("Animation").setField(registry, entity, "frameDuration", 0.25F);
    components.get("Animation").setField(registry, entity, "loop", false);
    const younity::render::Animation &animation = registry.getComponent<younity::render::Animation>(entity);

    cr_assert(animation.clip.frames == (std::vector<Recti>{
                                           Recti{0, 0, 8, 8},
                                           Recti{8, 0, 8, 8}
    }));
    cr_assert_float_eq(animation.clip.frameDuration, 0.25F, 0.0001F);
    cr_assert_not(animation.clip.loop);
    cr_assert(components.get("Animation").getField(registry, entity, "frames") ==
              FieldValue(std::vector<Recti>{
                  Recti{0, 0, 8, 8},
                  Recti{8, 0, 8, 8}
    }));
}

Test(render_components, write_the_texture_and_the_font_by_their_path)
{
    skipWithoutDisplay();

    ComponentRegistry components;
    ResourceManager resources = makeResources();
    Registry registry;
    const Entity entity = registry.createEntity();

    younity::render::registerComponents(components, resources);
    registry.addComponent(entity, Sprite{});
    registry.addComponent(entity, Text{});
    cr_assert(components.get("Sprite").getField(registry, entity, "texture") == FieldValue(std::string()));
    components.get("Sprite").setField(registry, entity, "texture", std::string("red_3x2.png"));
    components.get("Text").setField(registry, entity, "font", std::string("tuffy.ttf"));
    cr_assert_eq(registry.getComponent<Sprite>(entity).texture, *resources.load<Texture>("red_3x2.png"));
    cr_assert_eq(registry.getComponent<Text>(entity).font, *resources.load<Font>("tuffy.ttf"));
    cr_assert(components.get("Sprite").getField(registry, entity, "texture") == FieldValue(std::string("red_3x2.png")));
    components.get("Sprite").setField(registry, entity, "texture", std::string());
    cr_assert_null(registry.getComponent<Sprite>(entity).texture);
}

Test(render_components, a_texture_that_cannot_be_loaded_keeps_the_old_one)
{
    skipWithoutDisplay();

    ComponentRegistry components;
    ResourceManager resources = makeResources();
    Registry registry;
    const Entity entity = registry.createEntity();

    younity::render::registerComponents(components, resources);
    Sprite sprite;

    sprite.texture = *resources.load<Texture>("red_3x2.png");
    registry.addComponent(entity, sprite);
    cr_assert_throw(components.get("Sprite").setField(registry, entity, "texture", std::string("missing.png")), std::invalid_argument);
    cr_assert(components.get("Sprite").getField(registry, entity, "texture") == FieldValue(std::string("red_3x2.png")));
}

/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/Renderer
** File description:
** Renderer unit tests
*/

#include <criterion/criterion.h>

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "../TestSupport.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/render/AnimationClip.hpp"
#include "younity/render/Renderer.hpp"
#include "younity/render/Texture.hpp"
#include "younity/render/components/Animation.hpp"
#include "younity/render/components/Sprite.hpp"
#include "younity/render/components/Text.hpp"

using younity::Recti;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::render::Animation;
using younity::render::AnimationClip;
using younity::render::DrawItem;
using younity::render::DrawKind;
using younity::render::Sprite;
using younity::render::Text;
using younity::render::Texture;
using younity::test::skipWithoutDisplay;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "render" / "fixtures";

// A sprite of the 3 by 2 red image, so it covers 3 * scale by 2 * scale pixels
Entity addRedSprite(Registry &registry, const Transform &transform, int layer = 0)
{
    auto texture = Texture::loadFromFile(FIXTURES / "red_3x2.png");
    const Entity entity = registry.createEntity();
    Sprite sprite;

    cr_assert(texture.has_value());
    sprite.texture = std::make_shared<const Texture>(std::move(*texture));
    sprite.layer = layer;
    registry.addComponent(entity, transform);
    registry.addComponent(entity, sprite);
    return entity;
}

// The order needs no texture: the sprites of these tests have none, so they need no window either
Entity addSprite(Registry &registry, int layer)
{
    const Entity entity = registry.createEntity();
    Sprite sprite;

    sprite.layer = layer;
    registry.addComponent(entity, Transform{});
    registry.addComponent(entity, sprite);
    return entity;
}

// Texts need no font to be ordered either
Entity addText(Registry &registry, int layer)
{
    const Entity entity = registry.createEntity();
    Text text;

    text.string = "score";
    text.layer = layer;
    registry.addComponent(entity, Transform{});
    registry.addComponent(entity, text);
    return entity;
}

// The entities of the draw order, for the tests where every entity has a single drawable
std::vector<Entity> entitiesOf(const std::vector<DrawItem> &items)
{
    std::vector<Entity> entities;

    for (const DrawItem &item : items) {
        entities.push_back(item.entity);
    }
    return entities;
}

} // namespace

Test(render_renderer, empty_registry_draws_nothing)
{
    const Registry registry;

    cr_assert(younity::render::getDrawOrder(registry).empty());
}

Test(render_renderer, lower_layers_are_drawn_first)
{
    Registry registry;
    const Entity front = addSprite(registry, 2);
    const Entity back = addSprite(registry, -1);
    const Entity middle = addSprite(registry, 0);

    const std::vector<Entity> order = entitiesOf(younity::render::getDrawOrder(registry));
    const std::vector<Entity> expected{back, middle, front};
    cr_assert(order == expected);
}

Test(render_renderer, same_layer_is_drawn_by_entity_id)
{
    Registry registry;
    const Entity first = addSprite(registry, 1);
    const Entity second = addSprite(registry, 1);
    const Entity third = addSprite(registry, 1);

    const std::vector<Entity> order = entitiesOf(younity::render::getDrawOrder(registry));
    const std::vector<Entity> expected{first, second, third};
    cr_assert(order == expected);
}

Test(render_renderer, entity_without_transform_is_not_drawn)
{
    Registry registry;
    const Entity drawn = addSprite(registry, 0);
    const Entity floating = registry.createEntity();

    registry.addComponent(floating, Sprite{});
    const std::vector<Entity> order = entitiesOf(younity::render::getDrawOrder(registry));
    cr_assert_eq(order.size(), 1U);
    cr_assert_eq(order[0], drawn);
}

Test(render_renderer, hidden_sprite_is_not_drawn)
{
    Registry registry;
    const Entity shown = addSprite(registry, 0);
    const Entity hidden = addSprite(registry, 0);

    registry.getComponent<Sprite>(hidden).visible = false;
    const std::vector<Entity> order = entitiesOf(younity::render::getDrawOrder(registry));
    cr_assert_eq(order.size(), 1U);
    cr_assert_eq(order[0], shown);
}

Test(render_renderer, destroyed_entity_is_not_drawn)
{
    Registry registry;
    const Entity kept = addSprite(registry, 0);
    const Entity destroyed = addSprite(registry, 0);

    registry.destroyEntity(destroyed);
    const std::vector<Entity> order = entitiesOf(younity::render::getDrawOrder(registry));
    cr_assert_eq(order.size(), 1U);
    cr_assert_eq(order[0], kept);
}

Test(render_renderer, new_sprite_has_default_values)
{
    const Sprite sprite;

    cr_assert_null(sprite.texture);
    cr_assert_not(sprite.textureRect.has_value());
    cr_assert_eq(sprite.layer, 0);
    cr_assert(sprite.visible);
}

Test(render_renderer, texture_rect_is_the_one_of_the_sprite)
{
    Registry registry;
    const Entity entity = addSprite(registry, 0);

    registry.getComponent<Sprite>(entity).textureRect = Recti{1, 2, 3, 4};
    cr_assert(younity::render::getTextureRect(registry, entity) == (Recti{1, 2, 3, 4}));
}

Test(render_renderer, animation_frame_replaces_the_texture_rect)
{
    Registry registry;
    const Entity entity = addSprite(registry, 0);

    registry.getComponent<Sprite>(entity).textureRect = Recti{1, 2, 3, 4};
    registry.addComponent(entity, Animation{
                                      .clip = AnimationClip{{Recti{10, 0, 10, 10}}, 0.1F, true}
    });
    // Even before the AnimationSystem ran: a new entity never shows the whole sheet
    cr_assert(younity::render::getTextureRect(registry, entity) == (Recti{10, 0, 10, 10}));
}

Test(render_renderer, empty_animation_keeps_the_texture_rect)
{
    Registry registry;
    const Entity entity = addSprite(registry, 0);

    registry.getComponent<Sprite>(entity).textureRect = Recti{1, 2, 3, 4};
    registry.addComponent(entity, Animation{});
    cr_assert(younity::render::getTextureRect(registry, entity) == (Recti{1, 2, 3, 4}));
}

Test(render_renderer, texts_and_sprites_share_the_layers)
{
    Registry registry;
    const Entity score = addText(registry, 10);
    const Entity ship = addSprite(registry, 0);
    const Entity background = addSprite(registry, -1);
    const Entity label = addText(registry, 0);

    const std::vector<DrawItem> order = younity::render::getDrawOrder(registry);
    const std::vector<DrawItem> expected{
        {background, DrawKind::Sprite},
        {ship,       DrawKind::Sprite},
        {label,      DrawKind::Text  },
        {score,      DrawKind::Text  },
    };
    cr_assert(order == expected);
}

Test(render_renderer, sprite_of_an_entity_is_drawn_under_its_text)
{
    Registry registry;
    const Entity button = addSprite(registry, 5);

    registry.addComponent(button, Text{.font = nullptr, .string = "Play", .layer = 5});
    const std::vector<DrawItem> order = younity::render::getDrawOrder(registry);
    const std::vector<DrawItem> expected{
        {button, DrawKind::Sprite},
        {button, DrawKind::Text  }
    };
    cr_assert(order == expected);
}

Test(render_renderer, text_and_sprite_of_an_entity_keep_their_own_layer)
{
    Registry registry;
    const Entity entity = addSprite(registry, 3);

    registry.addComponent(entity, Text{.font = nullptr, .string = "behind", .layer = 1});
    const std::vector<DrawItem> order = younity::render::getDrawOrder(registry);
    const std::vector<DrawItem> expected{
        {entity, DrawKind::Text  },
        {entity, DrawKind::Sprite}
    };
    cr_assert(order == expected);
}

Test(render_renderer, hidden_text_is_not_drawn)
{
    Registry registry;
    const Entity hidden = addText(registry, 0);

    registry.getComponent<Text>(hidden).visible = false;
    cr_assert(younity::render::getDrawOrder(registry).empty());
}

Test(render_renderer, text_without_transform_is_not_drawn)
{
    Registry registry;
    const Entity floating = registry.createEntity();

    registry.addComponent(floating, Text{.font = nullptr, .string = "lost"});
    cr_assert(younity::render::getDrawOrder(registry).empty());
}

Test(render_renderer, draw_corners_follow_the_transform)
{
    skipWithoutDisplay();
    Registry registry;
    const Entity entity = addRedSprite(registry, Transform{
                                                     .position = {10.0F, 20.0F},
                                                     .scale = {2.0F,  2.0F }
    });

    const std::array<Vector2f, 4> corners = younity::render::getDrawCorners(registry, DrawItem{entity, DrawKind::Sprite});
    const std::array<Vector2f, 4> expected{
        Vector2f{10.0F, 20.0F},
        Vector2f{16.0F, 20.0F},
        Vector2f{16.0F, 24.0F},
        Vector2f{10.0F, 24.0F},
    };
    cr_assert(corners == expected);
}

Test(render_renderer, entity_at_a_point_of_its_sprite)
{
    skipWithoutDisplay();
    Registry registry;
    const Entity ship = addRedSprite(registry, Transform{
                                                   .position = {10.0F, 20.0F},
                                                   .scale = {10.0F, 10.0F}
    });

    cr_assert(younity::render::getEntityAt(registry, Vector2f{15.0F, 25.0F}) == ship);
    cr_assert(younity::render::getEntityAt(registry, Vector2f{39.0F, 39.0F}) == ship);
    cr_assert_not(younity::render::getEntityAt(registry, Vector2f{41.0F, 25.0F}).has_value());
    cr_assert_not(younity::render::getEntityAt(registry, Vector2f{5.0F, 25.0F}).has_value());
}

Test(render_renderer, entity_drawn_on_top_is_found)
{
    skipWithoutDisplay();
    Registry registry;
    const Entity front = addRedSprite(registry, Transform{
                                                    .scale = {10.0F, 10.0F}
    },
                                      1);

    addRedSprite(registry, Transform{
                               .scale = {10.0F, 10.0F}
    },
                 0);
    cr_assert(younity::render::getEntityAt(registry, Vector2f{5.0F, 5.0F}) == front);
}

Test(render_renderer, rotation_and_origin_turn_the_area)
{
    skipWithoutDisplay();
    Registry registry;
    const Entity entity = addRedSprite(registry, Transform{
                                                     .position = {100.0F, 100.0F},
                                                     .rotation = 90.0F,
                                                     .scale = {10.0F,  10.0F }
    });

    // Centered and turned a quarter: 20 pixels wide and 30 pixels high around the position
    registry.getComponent<Sprite>(entity).origin = Vector2f{1.5F, 1.0F};
    cr_assert(younity::render::getEntityAt(registry, Vector2f{105.0F, 112.0F}) == entity);
    cr_assert_not(younity::render::getEntityAt(registry, Vector2f{112.0F, 105.0F}).has_value());
}

Test(render_renderer, sprite_without_texture_is_not_at_any_point)
{
    Registry registry;

    addSprite(registry, 0);
    cr_assert_not(younity::render::getEntityAt(registry, Vector2f{0.0F, 0.0F}).has_value());
}

Test(render_renderer, hidden_sprite_is_not_at_any_point)
{
    skipWithoutDisplay();
    Registry registry;
    const Entity entity = addRedSprite(registry, Transform{
                                                     .scale = {10.0F, 10.0F}
    });

    registry.getComponent<Sprite>(entity).visible = false;
    cr_assert_not(younity::render::getEntityAt(registry, Vector2f{5.0F, 5.0F}).has_value());
}

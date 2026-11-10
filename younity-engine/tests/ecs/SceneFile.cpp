/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/SceneFile
** File description:
** SceneFile unit tests
*/

#include <criterion/criterion.h>

#include <expected>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "younity/ecs/SceneFile.hpp"
#include "younity/ecs/System.hpp"

using younity::Color;
using younity::Recti;
using younity::Vector2f;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::loadScene;
using younity::ecs::Registry;
using younity::ecs::saveScene;
using younity::ecs::SceneFileError;
using younity::ecs::System;

namespace {

struct Everything
{
    bool flag = false;
    int integer = 0;
    float decimal = 0.0F;
    std::string text;
    Vector2f point{};
    Color color{};
    std::optional<Recti> area;
    std::vector<Recti> frames;
};

struct Other
{
    int value = 0;
};

struct Clip
{
    std::vector<Recti> frames;
    float frameDuration = 0.1F;
    bool loop = true;
};

struct Animated
{
    Clip clip;
};

struct Hidden
{
    int value = 0;
};

// Counts the entities it updates, so a test sees which entities a system kept after a load
class CountingSystem : public System
{
public:
    explicit CountingSystem(int &count)
        : _count(count) {}

    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)registry;
        (void)dt;
        this->_count += static_cast<int>(entities.size());
    }

private:
    int &_count;
};

ComponentRegistry makeComponents(void)
{
    ComponentRegistry components;

    components.add<Everything>("Everything")
        .field("flag", &Everything::flag)
        .field("integer", &Everything::integer)
        .field("decimal", &Everything::decimal)
        .field("text", &Everything::text)
        .field("point", &Everything::point)
        .field("color", &Everything::color)
        .field("area", &Everything::area)
        .field("frames", &Everything::frames);
    components.add<Other>("Other").field("value", &Other::value);
    components.add<Animated>("Animated")
        .field("frames", &Animated::clip, &Clip::frames)
        .field("frameDuration", &Animated::clip, &Clip::frameDuration)
        .field("loop", &Animated::clip, &Clip::loop);
    return components;
}

std::string sceneWith(std::string_view entities)
{
    return "{\"format\":\"younity-scene\",\"version\":2,\"entities\":" + std::string(entities) + "}";
}

class TempFile
{
public:
    explicit TempFile(std::string_view name)
        : _path(std::filesystem::temp_directory_path() / makeName(name)) {}

    ~TempFile(void)
    {
        std::error_code error;

        std::filesystem::remove(this->_path, error);
    }

    TempFile(const TempFile &) = delete;
    TempFile &operator=(const TempFile &) = delete;

    [[nodiscard]] const std::filesystem::path &getPath(void) const noexcept
    {
        return this->_path;
    }

    void write(std::string_view content) const
    {
        std::ofstream output(this->_path, std::ios::binary | std::ios::trunc);

        output.write(content.data(), static_cast<std::streamsize>(content.size()));
        output.close();
        if (!output) {
            throw std::runtime_error("cannot write the test scene file");
        }
    }

    [[nodiscard]] std::string read(void) const
    {
        std::ifstream input(this->_path, std::ios::binary);

        if (!input.is_open()) {
            throw std::runtime_error("cannot read the test scene file");
        }
        return std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }

private:
    [[nodiscard]] static std::string makeName(std::string_view name)
    {
        static std::random_device random;

        return "younity-scene-" + std::to_string(random()) + "-" + std::to_string(random()) + "-" +
               std::string(name) + ".json";
    }

    std::filesystem::path _path;
};

void expectLoadFailure(
    const ComponentRegistry &components,
    const TempFile &file,
    std::string_view message,
    std::size_t line = 0)
{
    Registry registry;
    const std::expected<void, SceneFileError> result = loadScene(registry, components, file.getPath());

    cr_assert_not(result);
    cr_assert(result.error().path == file.getPath());
    cr_assert_eq(result.error().line, line);
    cr_assert_neq(result.error().message.find(message), std::string::npos, "message: %s", result.error().message.c_str());
    cr_assert(registry.getEntities().empty());
}

} // namespace

Test(ecs_scene_file, save_writes_the_format_and_the_registered_components)
{
    const ComponentRegistry components = makeComponents();
    Registry registry;
    const Entity first = registry.createEntity();
    Everything everything;

    everything.flag = true;
    everything.decimal = 0.1F;
    everything.text = "hello";
    everything.point = Vector2f{1.5F, -2.25F};
    everything.color = Color{1, 2, 3, 4};
    everything.area = Recti{5, 6, 7, 8};
    registry.addComponent(first, everything);
    const Entity second = registry.createEntity();

    registry.addComponent(second, Other{9});
    const Entity hidden = registry.createEntity();

    registry.addComponent(hidden, Hidden{42});
    TempFile file("save");

    cr_assert(saveScene(registry, components, file.getPath()));
    const std::string content = file.read();

    cr_assert_neq(content.find("\"format\": \"younity-scene\""), std::string::npos);
    cr_assert_neq(content.find("\"version\": 2"), std::string::npos);
    cr_assert_neq(content.find("\"id\": 2"), std::string::npos);
    cr_assert_neq(content.find("\"decimal\": 0.1"), std::string::npos);
    cr_assert_eq(content.find("0.100000001"), std::string::npos);
    cr_assert_neq(content.find("\"Everything\""), std::string::npos);
    cr_assert_neq(content.find("\"Other\""), std::string::npos);
    cr_assert(content.find("\"Everything\"") < content.find("\"Other\""));
    cr_assert(content.find("\"flag\"") < content.find("\"integer\""));
    cr_assert(content.find("\"integer\"") < content.find("\"decimal\""));
    cr_assert_eq(content.find("Hidden"), std::string::npos);
}

Test(ecs_scene_file, round_trip_keeps_every_field_type)
{
    const ComponentRegistry components = makeComponents();
    Registry saved;
    const Entity first = saved.createEntity();
    Everything everything;

    everything.flag = true;
    everything.integer = -42;
    everything.decimal = 0.1F;
    everything.text = "hello";
    everything.point = Vector2f{1.5F, -2.25F};
    everything.color = Color{1, 2, 3, 4};
    everything.area = Recti{5, 6, 7, 8};
    everything.frames = {
        Recti{0,  0, 16, 16},
        Recti{16, 0, 16, 16}
    };
    saved.addComponent(first, everything);
    const Entity second = saved.createEntity();

    saved.addComponent(second, Other{9});
    (void)saved.createEntity();
    TempFile file("round-trip");

    cr_assert(saveScene(saved, components, file.getPath()));
    Registry loaded;

    (void)loaded.createEntity();
    cr_assert(loadScene(loaded, components, file.getPath()));
    const std::vector<Entity> entities = loaded.getEntities();

    cr_assert_eq(entities.size(), 3U);
    cr_assert_eq(entities[0].getId(), 0U);
    cr_assert_eq(entities[1].getId(), 1U);
    cr_assert_eq(entities[2].getId(), 2U);
    cr_assert(loaded.hasComponent<Everything>(entities[0]));
    const Everything &loadedEverything = loaded.getComponent<Everything>(entities[0]);

    cr_assert_eq(loadedEverything.flag, true);
    cr_assert_eq(loadedEverything.integer, -42);
    cr_assert_float_eq(loadedEverything.decimal, 0.1F, 0.0001F);
    cr_assert_eq(loadedEverything.text, "hello");
    cr_assert(loadedEverything.point == (Vector2f{1.5F, -2.25F}));
    cr_assert_eq(loadedEverything.color, (Color{1, 2, 3, 4}));
    cr_assert(loadedEverything.area.has_value());
    cr_assert(*loadedEverything.area == (Recti{5, 6, 7, 8}));
    cr_assert(loadedEverything.frames == (std::vector<Recti>{
                                             Recti{0,  0, 16, 16},
                                             Recti{16, 0, 16, 16}
    }));
    cr_assert_eq(loaded.getComponent<Other>(entities[1]).value, 9);
    cr_assert_not(loaded.hasComponent<Everything>(entities[2]));
    cr_assert_not(loaded.hasComponent<Other>(entities[2]));
}

Test(ecs_scene_file, round_trip_keeps_an_animation_clip)
{
    const ComponentRegistry components = makeComponents();
    Registry saved;
    const Entity entity = saved.createEntity();
    Animated animated;

    animated.clip.frames = {
        Recti{0,  0, 16, 16},
        Recti{16, 0, 16, 16}
    };
    animated.clip.frameDuration = 0.25F;
    animated.clip.loop = false;
    saved.addComponent(entity, animated);
    TempFile file("animation-clip");

    cr_assert(saveScene(saved, components, file.getPath()));
    const std::string content = file.read();

    cr_assert_neq(content.find("\"frames\""), std::string::npos);
    cr_assert_neq(content.find("\"frameDuration\": 0.25"), std::string::npos);
    cr_assert_neq(content.find("\"loop\": false"), std::string::npos);
    Registry loaded;

    cr_assert(loadScene(loaded, components, file.getPath()));
    const Animated &loadedAnimated = loaded.getComponent<Animated>(loaded.getEntities()[0]);

    cr_assert(loadedAnimated.clip.frames == (std::vector<Recti>{
                                                Recti{0,  0, 16, 16},
                                                Recti{16, 0, 16, 16}
    }));
    cr_assert_float_eq(loadedAnimated.clip.frameDuration, 0.25F, 0.0001F);
    cr_assert_not(loadedAnimated.clip.loop);
}

Test(ecs_scene_file, load_keeps_the_ids_and_the_systems)
{
    const ComponentRegistry components = makeComponents();
    Registry registry;
    const Entity old = registry.createEntity();
    int updated = 0;

    registry.addComponent(old, Everything{});
    registry.addSystem<CountingSystem, Other>(updated);
    TempFile file("replace");

    file.write(sceneWith(R"([{"id":5,"components":{"Other":{"value":7}}}])"));
    cr_assert(loadScene(registry, components, file.getPath()));
    cr_assert_not(registry.isAlive(old));
    cr_assert(registry.getEntities() == std::vector<Entity>{Entity(5)});
    cr_assert_eq(registry.getComponent<Other>(Entity(5)).value, 7);
    registry.updateSystems(0.0F);
    cr_assert_eq(updated, 1);
}

Test(ecs_scene_file, save_then_load_gives_back_the_same_ids)
{
    const ComponentRegistry components = makeComponents();
    Registry registry;
    const Entity first = registry.createEntity();
    const Entity second = registry.createEntity();
    const Entity third = registry.createEntity();
    TempFile file("ids");

    registry.addComponent(third, Other{3});
    registry.destroyEntity(second);
    cr_assert(saveScene(registry, components, file.getPath()));
    cr_assert(loadScene(registry, components, file.getPath()));
    cr_assert(registry.getEntities() == (std::vector<Entity>{first, third}));
    cr_assert_eq(registry.getComponent<Other>(third).value, 3);
}

Test(ecs_scene_file, missing_fields_keep_their_default_value)
{
    const ComponentRegistry components = makeComponents();
    TempFile file("defaults");

    file.write(sceneWith(R"([{"id":0,"components":{"Everything":{}}}])"));
    Registry registry;

    cr_assert(loadScene(registry, components, file.getPath()));
    const Entity entity = registry.getEntities()[0];
    const Everything &everything = registry.getComponent<Everything>(entity);

    cr_assert_eq(everything.flag, false);
    cr_assert_eq(everything.integer, 0);
    cr_assert_float_eq(everything.decimal, 0.0F, 0.0001F);
    cr_assert(everything.text.empty());
    cr_assert(everything.point == Vector2f{});
    cr_assert(everything.color == Color{});
    cr_assert_not(everything.area.has_value());
    cr_assert(everything.frames.empty());
}

Test(ecs_scene_file, load_rejects_an_unknown_component_or_field)
{
    const ComponentRegistry components = makeComponents();
    TempFile componentFile("unknown-component");

    componentFile.write(sceneWith(R"([{"id":0,"components":{"Unknown":{}}}])"));
    expectLoadFailure(components, componentFile, "unknown component");
    TempFile fieldFile("unknown-field");

    fieldFile.write(sceneWith(R"([{"id":0,"components":{"Everything":{"unknown":1}}}])"));
    expectLoadFailure(components, fieldFile, "unknown field");
}

Test(ecs_scene_file, load_rejects_invalid_values)
{
    const ComponentRegistry components = makeComponents();
    const std::vector<std::pair<std::string, std::string>> cases = {
        {R"([{"id":0,"components":{"Everything":{"flag":1}}}])",               "expected a boolean"                        },
        {R"([{"id":0,"components":{"Everything":{"integer":1.5}}}])",          "expected an integer"                       },
        {R"([{"id":0,"components":{"Everything":{"integer":2147483648}}}])",   "out of the range of an int"                },
        {R"([{"id":0,"components":{"Everything":{"decimal":"fast"}}}])",       "expected a number"                         },
        {R"([{"id":0,"components":{"Everything":{"text":12}}}])",              "expected a string"                         },
        {R"([{"id":0,"components":{"Everything":{"point":[1]}}}])",            "expected an array of two numbers"          },
        {R"([{"id":0,"components":{"Everything":{"color":[1,2,3]}}}])",        "expected an array of four integers"        },
        {R"([{"id":0,"components":{"Everything":{"color":[1,2,3,256]}}}])",    "out of the range 0 to 255"                 },
        {R"([{"id":0,"components":{"Everything":{"area":12}}}])",              "expected null or an array of four integers"},
        {R"([{"id":0,"components":{"Everything":{"frames":12}}}])",            "expected an array of rectangles"           },
        {R"([{"id":0,"components":{"Everything":{"frames":[[1,2,3]]}}}])",     "expected an array of four integers"        },
        {R"([{"id":0,"components":{"Everything":{"frames":[[1,2,3,"x"]]}}}])", "expected an integer"                       },
    };

    for (const auto &[entities, message] : cases) {
        TempFile file("invalid-value");

        file.write(sceneWith(entities));
        expectLoadFailure(components, file, message);
    }
}

Test(ecs_scene_file, load_rejects_unknown_or_missing_top_level_data)
{
    const ComponentRegistry components = makeComponents();
    const std::vector<std::pair<std::string, std::string>> cases = {
        {R"([])",                                                                                                    "must be a JSON object"     },
        {R"({"version":2,"entities":[]})",                                                                           "no format"                 },
        {R"({"format":"other","version":2,"entities":[]})",                                                          "format must be"            },
        {R"({"format":"younity-scene","entities":[]})",                                                              "no version"                },
        {R"({"format":"younity-scene","version":"1","entities":[]})",                                                "version must be an integer"},
        {R"({"format":"younity-scene","version":1,"entities":[]})",                                                  "unsupported scene version" },
        {R"({"format":"younity-scene","version":2})",                                                                "no entities"               },
        {R"({"format":"younity-scene","version":2,"entities":{}})",                                                  "entities must be an array" },
        {R"({"format":"younity-scene","version":2,"entities":[],"extra":0})",                                        "unknown top-level key"     },
        {R"({"format":"younity-scene","version":2,"entities":[1]})",                                                 "expected an object"        },
        {R"({"format":"younity-scene","version":2,"entities":[{}]})",                                                "no id"                     },
        {R"({"format":"younity-scene","version":2,"entities":[{"id":0}]})",                                          "no components"             },
        {R"({"format":"younity-scene","version":2,"entities":[{"id":"0","components":{}}]})",                        "expected an integer from 0"},
        {R"({"format":"younity-scene","version":2,"entities":[{"id":-1,"components":{}}]})",                         "expected an integer from 0"},
        {R"({"format":"younity-scene","version":2,"entities":[{"id":1048576,"components":{}}]})",                    "expected an integer from 0"},
        {R"({"format":"younity-scene","version":2,"entities":[{"id":3,"components":{}},{"id":3,"components":{}}]})", "used twice"                },
        {R"({"format":"younity-scene","version":2,"entities":[{"id":0,"components":{},"extra":1}]})",                "unknown key"               },
        {R"({"format":"younity-scene","version":2,"entities":[{"id":0,"components":[]}]})",                          "expected an object"        },
    };

    for (const auto &[content, message] : cases) {
        TempFile file("invalid-scene");

        file.write(content);
        expectLoadFailure(components, file, message);
    }
}

Test(ecs_scene_file, load_reports_the_line_of_a_syntax_error)
{
    const ComponentRegistry components = makeComponents();
    TempFile file("syntax");

    file.write("{\n  \"format\": \"younity-scene\",\n  \"version\": 2,\n  \"entities\": ]\n}\n");
    expectLoadFailure(components, file, "invalid JSON", 4U);
}

Test(ecs_scene_file, invalid_file_leaves_the_old_scene_untouched)
{
    const ComponentRegistry components = makeComponents();
    Registry registry;
    const Entity old = registry.createEntity();
    Everything everything;

    everything.integer = 7;
    registry.addComponent(old, everything);
    int updates = 0;

    registry.addSystem<CountingSystem>(updates);
    TempFile file("rollback");

    file.write(sceneWith(R"([{"id":0,"components":{"Everything":{"integer":"seven"}}}])"));
    const std::expected<void, SceneFileError> result = loadScene(registry, components, file.getPath());

    cr_assert_not(result);
    cr_assert(registry.isAlive(old));
    cr_assert_eq(registry.getComponent<Everything>(old).integer, 7);
    registry.updateSystems(0.0F);
    cr_assert_eq(updates, 1);
}

Test(ecs_scene_file, duplicate_json_keys_keep_the_last_value)
{
    const ComponentRegistry components = makeComponents();
    TempFile file("duplicate");

    file.write(sceneWith(R"([{"id":0,"components":{"Everything":{"integer":1,"integer":2}}}])"));
    Registry registry;

    cr_assert(loadScene(registry, components, file.getPath()));
    cr_assert_eq(registry.getComponent<Everything>(registry.getEntities()[0]).integer, 2);
}

Test(ecs_scene_file, load_reports_a_missing_file)
{
    const ComponentRegistry components = makeComponents();
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "younity-scene-does-not-exist.json";
    Registry registry;
    const std::expected<void, SceneFileError> result = loadScene(registry, components, path);

    cr_assert_not(result);
    cr_assert(result.error().path == path);
    cr_assert_eq(result.error().line, 0U);
    cr_assert_neq(result.error().message.find("cannot open"), std::string::npos);
}

Test(ecs_scene_file, save_reports_a_folder_that_does_not_exist)
{
    const ComponentRegistry components = makeComponents();
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "younity-scene-missing-folder" / "scene.json";
    Registry registry;
    const std::expected<void, SceneFileError> result = saveScene(registry, components, path);

    cr_assert_not(result);
    cr_assert(result.error().path == path);
    cr_assert_eq(result.error().line, 0U);
    cr_assert_neq(result.error().message.find("cannot open"), std::string::npos);
}

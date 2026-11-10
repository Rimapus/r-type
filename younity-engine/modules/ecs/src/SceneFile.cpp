/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/src/SceneFile
** File description:
** SceneFile source file
*/

#include "younity/ecs/SceneFile.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

namespace younity::ecs {

namespace {

// Floats are kept as float by the parser and the writer, so a scene file shows 0.1 instead of
// 0.10000000149011612, and a number too large for a float is refused when the file is parsed.
using Json = nlohmann::basic_json<nlohmann::ordered_map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;

constexpr std::string_view SCENE_FORMAT = "younity-scene";
constexpr std::int64_t SCENE_VERSION = 2;

[[nodiscard]] SceneFileError makeError(
    const std::filesystem::path &path,
    std::size_t line,
    std::string message)
{
    return SceneFileError{path, line, std::move(message)};
}

[[nodiscard]] std::size_t lineOf(const std::string &content, std::size_t byte)
{
    if (byte == 0) {
        return 0;
    }
    const std::size_t end = std::min(byte, content.size());
    const std::string_view prefix(content.data(), end);

    return 1 + static_cast<std::size_t>(std::count(prefix.begin(), prefix.end(), '\n'));
}

[[nodiscard]] bool isAllowedKey(const std::string &key, std::initializer_list<std::string_view> allowed)
{
    for (const std::string_view name : allowed) {
        if (key.compare(name) == 0) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] std::expected<int, std::string> parseInteger(const Json &value)
{
    if (!value.is_number_integer()) {
        return std::unexpected("expected an integer");
    }
    try {
        const std::int64_t number = value.get<std::int64_t>();

        if (!std::in_range<int>(number)) {
            return std::unexpected("the integer is out of the range of an int");
        }
        return static_cast<int>(number);
    } catch (const Json::out_of_range &) {
        return std::unexpected("the integer is out of the range of an int");
    }
}

[[nodiscard]] std::expected<float, std::string> parseFloat(const Json &value)
{
    if (!value.is_number()) {
        return std::unexpected("expected a number");
    }
    try {
        const float number = value.get<float>();

        if (!std::isfinite(number)) {
            return std::unexpected("the number is out of the range of a float");
        }
        return number;
    } catch (const Json::out_of_range &) {
        return std::unexpected("the number is out of the range of a float");
    }
}

[[nodiscard]] std::expected<Recti, std::string> parseRecti(const Json &value)
{
    if (!value.is_array() || value.size() != 4) {
        return std::unexpected("expected an array of four integers");
    }
    std::expected<int, std::string> left = parseInteger(value[0]);
    std::expected<int, std::string> top = parseInteger(value[1]);
    std::expected<int, std::string> width = parseInteger(value[2]);
    std::expected<int, std::string> height = parseInteger(value[3]);

    if (!left) {
        return std::unexpected(left.error());
    }
    if (!top) {
        return std::unexpected(top.error());
    }
    if (!width) {
        return std::unexpected(width.error());
    }
    if (!height) {
        return std::unexpected(height.error());
    }
    return Recti{*left, *top, *width, *height};
}

[[nodiscard]] std::expected<FieldValue, std::string> parseFieldValue(const Json &value, FieldType type)
{
    switch (type) {
    case FieldType::Bool:
        if (!value.is_boolean()) {
            return std::unexpected("expected a boolean");
        }
        return FieldValue{value.get<bool>()};
    case FieldType::Int: {
        std::expected<int, std::string> number = parseInteger(value);

        if (!number) {
            return std::unexpected(number.error());
        }
        return FieldValue{*number};
    }
    case FieldType::Float: {
        std::expected<float, std::string> number = parseFloat(value);

        if (!number) {
            return std::unexpected(number.error());
        }
        return FieldValue{*number};
    }
    case FieldType::String:
        if (!value.is_string()) {
            return std::unexpected("expected a string");
        }
        return FieldValue{value.get<std::string>()};
    case FieldType::Vector2f: {
        if (!value.is_array() || value.size() != 2) {
            return std::unexpected("expected an array of two numbers");
        }
        std::expected<float, std::string> x = parseFloat(value[0]);
        std::expected<float, std::string> y = parseFloat(value[1]);

        if (!x) {
            return std::unexpected(x.error());
        }
        if (!y) {
            return std::unexpected(y.error());
        }
        return FieldValue{
            Vector2f{*x, *y}
        };
    }
    case FieldType::Color: {
        if (!value.is_array() || value.size() != 4) {
            return std::unexpected("expected an array of four integers from 0 to 255");
        }
        Color color;
        std::uint8_t *parts[4] = {&color.r, &color.g, &color.b, &color.a};

        for (std::size_t index = 0; index < 4; ++index) {
            std::expected<int, std::string> part = parseInteger(value[index]);

            if (!part) {
                return std::unexpected(part.error());
            }
            if (*part < 0 || *part > 255) {
                return std::unexpected("a color part is out of the range 0 to 255");
            }
            *parts[index] = static_cast<std::uint8_t>(*part);
        }
        return FieldValue{color};
    }
    case FieldType::OptionalRecti: {
        if (value.is_null()) {
            return FieldValue{std::optional<Recti>{}};
        }
        if (!value.is_array() || value.size() != 4) {
            return std::unexpected("expected null or an array of four integers");
        }
        std::expected<Recti, std::string> rect = parseRecti(value);

        if (!rect) {
            return std::unexpected(rect.error());
        }
        return FieldValue{std::optional<Recti>{*rect}};
    }
    case FieldType::RectiVector: {
        if (!value.is_array()) {
            return std::unexpected("expected an array of rectangles");
        }
        std::vector<Recti> frames;

        frames.reserve(value.size());
        for (const Json &frame : value) {
            std::expected<Recti, std::string> rect = parseRecti(frame);

            if (!rect) {
                return std::unexpected(rect.error());
            }
            frames.push_back(*rect);
        }
        return FieldValue{std::move(frames)};
    }
    }
    return std::unexpected("unknown field type");
}

[[nodiscard]] Json fieldValueToJson(const FieldValue &value)
{
    return std::visit(
        [](const auto &field) -> Json {
            using Type = std::decay_t<decltype(field)>;

            if constexpr (std::same_as<Type, bool>) {
                return field;
            } else if constexpr (std::same_as<Type, int>) {
                return field;
            } else if constexpr (std::same_as<Type, float>) {
                return field;
            } else if constexpr (std::same_as<Type, std::string>) {
                return field;
            } else if constexpr (std::same_as<Type, Vector2f>) {
                return Json::array({field.x, field.y});
            } else if constexpr (std::same_as<Type, Color>) {
                return Json::array({
                    static_cast<int>(field.r),
                    static_cast<int>(field.g),
                    static_cast<int>(field.b),
                    static_cast<int>(field.a),
                });
            } else if constexpr (std::same_as<Type, std::vector<Recti>>) {
                Json frames = Json::array();

                for (const Recti &frame : field) {
                    frames.push_back(Json::array({frame.left, frame.top, frame.width, frame.height}));
                }
                return frames;
            } else {
                if (!field.has_value()) {
                    return Json{};
                }
                return Json::array({field->left, field->top, field->width, field->height});
            }
        },
        value);
}

[[nodiscard]] std::expected<void, SceneFileError> buildRegistry(
    const Json &root,
    const ComponentRegistry &components,
    const std::filesystem::path &path,
    Registry &registry)
{
    if (!root.is_object()) {
        return std::unexpected(makeError(path, 0, "the scene must be a JSON object"));
    }
    for (auto it = root.begin(); it != root.end(); ++it) {
        if (!isAllowedKey(it.key(), {"format", "version", "entities"})) {
            return std::unexpected(makeError(path, 0, "unknown top-level key \"" + it.key() + "\""));
        }
    }
    const auto format = root.find("format");

    if (format == root.end()) {
        return std::unexpected(makeError(path, 0, "the scene has no format"));
    }
    if (!format->is_string() || format->get<std::string>().compare(SCENE_FORMAT) != 0) {
        return std::unexpected(makeError(path, 0, "the format must be \"" + std::string(SCENE_FORMAT) + "\""));
    }
    const auto version = root.find("version");

    if (version == root.end()) {
        return std::unexpected(makeError(path, 0, "the scene has no version"));
    }
    if (!version->is_number_integer()) {
        return std::unexpected(makeError(path, 0, "the version must be an integer"));
    }
    if (*version != SCENE_VERSION) {
        return std::unexpected(makeError(path, 0, "unsupported scene version " + version->dump()));
    }
    const auto entities = root.find("entities");

    if (entities == root.end()) {
        return std::unexpected(makeError(path, 0, "the scene has no entities"));
    }
    if (!entities->is_array()) {
        return std::unexpected(makeError(path, 0, "the entities must be an array"));
    }
    for (std::size_t entityIndex = 0; entityIndex < entities->size(); ++entityIndex) {
        const Json &entityJson = (*entities)[entityIndex];
        const std::string entityPath = "entities[" + std::to_string(entityIndex) + "]";

        if (!entityJson.is_object()) {
            return std::unexpected(makeError(path, 0, entityPath + ": expected an object"));
        }
        for (auto it = entityJson.begin(); it != entityJson.end(); ++it) {
            if (!isAllowedKey(it.key(), {"id", "components"})) {
                return std::unexpected(makeError(path, 0, entityPath + ": unknown key \"" + it.key() + "\""));
            }
        }
        const auto id = entityJson.find("id");

        if (id == entityJson.end()) {
            return std::unexpected(makeError(path, 0, entityPath + ": the entity has no id"));
        }
        const std::expected<int, std::string> number = parseInteger(*id);

        if (!number || *number < 0 || static_cast<Entity::Id>(*number) > MAX_SCENE_ENTITY_ID) {
            return std::unexpected(makeError(path, 0, entityPath + ".id: expected an integer from 0 to " + std::to_string(MAX_SCENE_ENTITY_ID)));
        }
        if (registry.isAlive(Entity(static_cast<Entity::Id>(*number)))) {
            return std::unexpected(makeError(path, 0, entityPath + ".id: the id " + std::to_string(*number) + " is used twice"));
        }
        const auto componentsJson = entityJson.find("components");

        if (componentsJson == entityJson.end()) {
            return std::unexpected(makeError(path, 0, entityPath + ": the entity has no components"));
        }
        if (!componentsJson->is_object()) {
            return std::unexpected(makeError(path, 0, entityPath + ".components: expected an object"));
        }
        const Entity entity = registry.createEntity(static_cast<Entity::Id>(*number));

        for (auto componentIt = componentsJson->begin(); componentIt != componentsJson->end(); ++componentIt) {
            const std::string componentPath = entityPath + ".components." + componentIt.key();
            const ComponentInfo *info = nullptr;

            try {
                info = &components.get(componentIt.key());
            } catch (const std::out_of_range &) {
                return std::unexpected(makeError(path, 0, componentPath + ": unknown component"));
            }
            const Json &fields = componentIt.value();

            if (!fields.is_object()) {
                return std::unexpected(makeError(path, 0, componentPath + ": expected an object"));
            }
            try {
                info->add(registry, entity);
            } catch (const std::length_error &error) {
                return std::unexpected(makeError(path, 0, componentPath + ": " + error.what()));
            }
            for (auto fieldIt = fields.begin(); fieldIt != fields.end(); ++fieldIt) {
                const std::string fieldPath = componentPath + "." + fieldIt.key();
                FieldType type = FieldType::Bool;

                try {
                    type = info->getFieldType(fieldIt.key());
                } catch (const std::out_of_range &) {
                    return std::unexpected(makeError(path, 0, fieldPath + ": unknown field"));
                }
                std::expected<FieldValue, std::string> value = parseFieldValue(fieldIt.value(), type);

                if (!value) {
                    return std::unexpected(makeError(path, 0, fieldPath + ": " + value.error()));
                }
                try {
                    info->setField(registry, entity, fieldIt.key(), *value);
                } catch (const std::out_of_range &) {
                    return std::unexpected(makeError(path, 0, fieldPath + ": the value is out of the range of the field"));
                } catch (const std::invalid_argument &error) {
                    return std::unexpected(makeError(path, 0, fieldPath + ": " + error.what()));
                }
            }
        }
    }
    return {};
}

} // namespace

std::expected<void, SceneFileError> saveScene(
    const Registry &registry,
    const ComponentRegistry &components,
    const std::filesystem::path &path)
{
    Json root = Json::object();

    root["format"] = std::string(SCENE_FORMAT);
    root["version"] = SCENE_VERSION;
    root["entities"] = Json::array();
    for (const Entity entity : registry.getEntities()) {
        if (entity.getId() > MAX_SCENE_ENTITY_ID) {
            return std::unexpected(makeError(path, 0, "the entity " + std::to_string(entity.getId()) + " has an id above " + std::to_string(MAX_SCENE_ENTITY_ID)));
        }
        Json componentsJson = Json::object();

        for (const std::string &name : components.getNames()) {
            const ComponentInfo &info = components.get(name);

            if (!info.has(registry, entity)) {
                continue;
            }
            Json fields = Json::object();

            for (const std::string &field : info.getFieldNames()) {
                fields[field] = fieldValueToJson(info.getField(registry, entity, field));
            }
            componentsJson[name] = std::move(fields);
        }
        Json entityJson = Json::object();

        entityJson["id"] = entity.getId();
        entityJson["components"] = std::move(componentsJson);
        root["entities"].push_back(std::move(entityJson));
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);

    if (!output.is_open()) {
        return std::unexpected(makeError(path, 0, "cannot open the scene file for writing"));
    }
    output << root.dump(2) << '\n';
    output.close();
    if (!output) {
        return std::unexpected(makeError(path, 0, "cannot write the scene file"));
    }
    return {};
}

std::expected<void, SceneFileError> loadScene(
    Registry &registry,
    const ComponentRegistry &components,
    const std::filesystem::path &path)
{
    std::ifstream input(path, std::ios::binary);

    if (!input.is_open()) {
        return std::unexpected(makeError(path, 0, "cannot open the scene file for reading"));
    }
    const std::string content{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};

    if (input.bad()) {
        return std::unexpected(makeError(path, 0, "cannot read the scene file"));
    }
    Json root;

    try {
        root = Json::parse(content);
    } catch (const Json::parse_error &error) {
        return std::unexpected(makeError(path, lineOf(content, error.byte), "invalid JSON: " + std::string(error.what())));
    } catch (const Json::out_of_range &error) {
        return std::unexpected(makeError(path, 0, "invalid JSON: " + std::string(error.what())));
    } catch (const std::exception &error) {
        return std::unexpected(makeError(path, 0, "invalid JSON: " + std::string(error.what())));
    }
    Registry scratch;
    const std::expected<void, SceneFileError> checked = buildRegistry(root, components, path, scratch);

    if (!checked) {
        return checked;
    }
    // Built again in place rather than moved from the scratch registry, so the registry keeps its systems
    registry.destroyEntities();
    return buildRegistry(root, components, path, registry);
}

} // namespace younity::ecs

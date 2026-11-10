/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/ComponentInfo
** File description:
** ComponentInfo header file
*/

#pragma once

#include <concepts>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/FieldValue.hpp"
#include "younity/ecs/Registry.hpp"

namespace younity::ecs {

/**
 * @brief A component type known by its name: its fields, and the functions to add it to an entity,
 *        remove it and read or write its fields without knowing its C++ type.
 *        It is created by ComponentRegistry::add().
 */
class ComponentInfo
{
public:
    /**
     * @brief Create the information of a component type, without any field.
     *
     * @tparam Component The type of the component, built with its default values when added by name
     * @param name The name of the component
     *
     * @return The information of the component type.
     */
    template <std::default_initializable Component>
    [[nodiscard]] static ComponentInfo create(std::string name)
    {
        ComponentInfo info(std::move(name), std::type_index(typeid(Component)));

        info._has = [](const Registry &registry, Entity entity) { return registry.hasComponent<Component>(entity); };
        info._add = [](Registry &registry, Entity entity) { registry.addComponent(entity, Component{}); };
        info._remove = [](Registry &registry, Entity entity) { registry.removeComponent<Component>(entity); };
        return info;
    }

    /**
     * @brief Add a field of the component, read and written by its name,
     *        e.g. info.field("rotation", &Transform::rotation).
     *        An integer field of any size is read and written as an int.
     *
     * @tparam Component The type of the component, the one given to create()
     * @tparam Member The type of the field, one of the types of FieldValue
     * @param name The name of the field
     * @param member The field in the component
     *
     * @return This information, to add the next field.
     *
     * @throw std::invalid_argument if the field belongs to another component or the name is already used.
     */
    template <typename Component, typename Member>
    ComponentInfo &field(std::string name, Member Component::*member)
    {
        return this->field<Component, Member>(
            std::move(name),
            [member](const Component &component) -> Member { return component.*member; },
            [member](Component &component, Member value) { component.*member = std::move(value); });
    }

    /**
     * @brief Add a field nested in the component, read and written by its name,
     *        e.g. info.field("frames", &Animation::clip, &AnimationClip::frames).
     *
     * @tparam Component The type of the component, the one given to create()
     * @tparam Nested The type of the member holding the field
     * @tparam Member The type of the field, one of the types of FieldValue
     * @param name The name of the field
     * @param nested The member holding the field in the component
     * @param member The field in the nested member
     *
     * @return This information, to add the next field.
     *
     * @throw std::invalid_argument if the field belongs to another component or the name is already used.
     */
    template <typename Component, typename Nested, typename Member>
    ComponentInfo &field(std::string name, Nested Component::*nested, Member Nested::*member)
    {
        return this->field<Component, Member>(
            std::move(name),
            [nested, member](const Component &component) -> Member { return component.*nested.*member; },
            [nested, member](Component &component, Member value) {
                component.*nested.*member = std::move(value);
            });
    }

    /**
     * @brief Add a field read and written through functions, for a value the component does not hold as is,
     *        e.g. info.field<Sprite, std::string>("texture", pathOfTexture, loadTexture).
     *        The setter may throw std::invalid_argument to refuse a value: the field keeps the one it had.
     *
     * @tparam Component The type of the component, the one given to create()
     * @tparam Member The type of the field, one of the types of FieldValue
     * @param name The name of the field
     * @param get Read the field of a component
     * @param set Write the field of a component
     *
     * @return This information, to add the next field.
     *
     * @throw std::invalid_argument if the field belongs to another component or the name is already used.
     */
    template <typename Component, typename Member>
    ComponentInfo &field(
        std::string name,
        std::function<Member(const Component &)> get,
        std::function<void(Component &, Member)> set)
    {
        using Exposed = std::conditional_t<std::integral<Member> && !std::same_as<Member, bool>, int, Member>;

        static_assert(isFieldType<Exposed>(std::type_identity<FieldValue>{}), "the type of the field is missing from FieldValue");
        if (std::type_index(typeid(Component)) != this->_type) {
            throw std::invalid_argument("younity::ecs::ComponentInfo: the field belongs to another component");
        }
        if (this->findField(name) != nullptr) {
            throw std::invalid_argument("younity::ecs::ComponentInfo: the field " + name + " already exists");
        }
        this->_fields.push_back(Field{
            std::move(name),
            fieldTypeOf<Exposed>(),
            [get = std::move(get)](const Registry &registry, Entity entity) -> FieldValue {
                return static_cast<Exposed>(get(registry.getComponent<Component>(entity)));
            },
            [set = std::move(set)](Registry &registry, Entity entity, const FieldValue &value) {
                const Exposed *exposed = std::get_if<Exposed>(&value);

                if (exposed == nullptr) {
                    throw std::invalid_argument("younity::ecs::ComponentInfo: the value has another type than the field");
                }
                if constexpr (!std::same_as<Exposed, Member>) {
                    if (!std::in_range<Member>(*exposed)) {
                        throw std::out_of_range("younity::ecs::ComponentInfo: the value is out of the range of the field");
                    }
                }
                set(registry.getComponent<Component>(entity), static_cast<Member>(*exposed));
            },
        });
        return *this;
    }

    /**
     * @brief Get the name of the component.
     *
     * @return The name given to ComponentRegistry::add().
     */
    [[nodiscard]] const std::string &getName(void) const noexcept;

    /**
     * @brief Get the C++ type of the component.
     *
     * @return The type of the component.
     */
    [[nodiscard]] std::type_index getType(void) const noexcept;

    /**
     * @brief Get the names of the fields of the component.
     *
     * @return The names, in the order the fields were added.
     */
    [[nodiscard]] std::vector<std::string> getFieldNames(void) const;

    /**
     * @brief Get the type of a field of the component.
     *
     * @param name The name of the field
     *
     * @return The type of the field, one of the types of FieldValue.
     *
     * @throw std::out_of_range if the component has no such field.
     */
    [[nodiscard]] FieldType getFieldType(std::string_view name) const;

    /**
     * @brief Check if the entity has this component.
     *
     * @param registry The registry of the entity
     * @param entity The entity to check
     *
     * @return true if the entity has this component.
     */
    [[nodiscard]] bool has(const Registry &registry, Entity entity) const;

    /**
     * @brief Add this component to the entity, with its default values,
     *        replacing the one it already has.
     *
     * @param registry The registry of the entity
     * @param entity The entity receiving the component
     *
     * @throw std::invalid_argument if the entity is not alive.
     */
    void add(Registry &registry, Entity entity) const;

    /**
     * @brief Remove this component from the entity, does nothing if it has none.
     *
     * @param registry The registry of the entity
     * @param entity The entity owning the component
     *
     * @throw std::invalid_argument if the entity is not alive.
     */
    void remove(Registry &registry, Entity entity) const;

    /**
     * @brief Read a field of the component of the entity.
     *
     * @param registry The registry of the entity
     * @param entity The entity owning the component
     * @param name The name of the field
     *
     * @return The value of the field.
     *
     * @throw std::out_of_range if the component has no such field, or the entity has no such component.
     * @throw std::invalid_argument if the entity is not alive.
     */
    [[nodiscard]] FieldValue getField(const Registry &registry, Entity entity, std::string_view name) const;

    /**
     * @brief Write a field of the component of the entity.
     *
     * @param registry The registry of the entity
     * @param entity The entity owning the component
     * @param name The name of the field
     * @param value The new value, of the type of the field
     *
     * @throw std::out_of_range if the component has no such field, the entity has no such component,
     *        or the value is out of the range of an integer field.
     * @throw std::invalid_argument if the value has another type than the field, the setter of the field refuses it,
     *        or the entity is not alive.
     */
    void setField(Registry &registry, Entity entity, std::string_view name, const FieldValue &value) const;

private:
    /**
     * @brief A field of the component, and the functions to read and write it.
     */
    struct Field
    {
        std::string name;
        FieldType type;
        std::function<FieldValue(const Registry &, Entity)> get;
        std::function<void(Registry &, Entity, const FieldValue &)> set;
    };

    /**
     * @brief Create the information of a component type, its functions are set by create().
     *
     * @param name The name of the component
     * @param type The C++ type of the component
     */
    ComponentInfo(std::string name, std::type_index type);

    /**
     * @brief Find a field by its name.
     *
     * @param name The name of the field
     *
     * @return The field, or nullptr if the component has no such field.
     */
    [[nodiscard]] const Field *findField(std::string_view name) const noexcept;

    /**
     * @brief Get a field by its name.
     *
     * @param name The name of the field
     *
     * @return The field.
     *
     * @throw std::out_of_range if the component has no such field.
     */
    [[nodiscard]] const Field &getFieldEntry(std::string_view name) const;

    /**
     * @brief Check if a type is one of the types of FieldValue.
     *
     * @tparam Type The type to check
     * @tparam Types The types of FieldValue
     *
     * @return true if Type is one of them.
     */
    template <typename Type, typename... Types>
    static consteval bool isFieldType(std::type_identity<std::variant<Types...>> /* fieldValue */)
    {
        return (std::same_as<Type, Types> || ...);
    }

    /**
     * @brief Get the FieldType of a type of FieldValue.
     *
     * @tparam Type The type of the field
     *
     * @return The type of the field.
     */
    template <typename Type>
    static consteval FieldType fieldTypeOf(void)
    {
        if constexpr (std::same_as<Type, bool>) {
            return FieldType::Bool;
        } else if constexpr (std::same_as<Type, int>) {
            return FieldType::Int;
        } else if constexpr (std::same_as<Type, float>) {
            return FieldType::Float;
        } else if constexpr (std::same_as<Type, std::string>) {
            return FieldType::String;
        } else if constexpr (std::same_as<Type, Vector2f>) {
            return FieldType::Vector2f;
        } else if constexpr (std::same_as<Type, Color>) {
            return FieldType::Color;
        } else if constexpr (std::same_as<Type, std::vector<Recti>>) {
            return FieldType::RectiVector;
        } else {
            static_assert(std::same_as<Type, std::optional<Recti>>, "the FieldType of this field is missing");
            return FieldType::OptionalRecti;
        }
    }

    std::string _name;
    std::type_index _type;
    std::function<bool(const Registry &, Entity)> _has;
    std::function<void(Registry &, Entity)> _add;
    std::function<void(Registry &, Entity)> _remove;
    std::vector<Field> _fields;
};

} // namespace younity::ecs

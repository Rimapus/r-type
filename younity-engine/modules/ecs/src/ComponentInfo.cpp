/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/src/ComponentInfo
** File description:
** ComponentInfo source file
*/

#include "younity/ecs/ComponentInfo.hpp"

namespace younity::ecs {

ComponentInfo::ComponentInfo(std::string name, std::type_index type)
    : _name(std::move(name)), _type(type)
{
}

const std::string &ComponentInfo::getName(void) const noexcept
{
    return this->_name;
}

std::type_index ComponentInfo::getType(void) const noexcept
{
    return this->_type;
}

std::vector<std::string> ComponentInfo::getFieldNames(void) const
{
    std::vector<std::string> names;

    names.reserve(this->_fields.size());
    for (const Field &field : this->_fields) {
        names.push_back(field.name);
    }
    return names;
}

FieldType ComponentInfo::getFieldType(std::string_view name) const
{
    return this->getFieldEntry(name).type;
}

bool ComponentInfo::has(const Registry &registry, Entity entity) const
{
    return this->_has(registry, entity);
}

void ComponentInfo::add(Registry &registry, Entity entity) const
{
    this->_add(registry, entity);
}

void ComponentInfo::remove(Registry &registry, Entity entity) const
{
    this->_remove(registry, entity);
}

FieldValue ComponentInfo::getField(const Registry &registry, Entity entity, std::string_view name) const
{
    return this->getFieldEntry(name).get(registry, entity);
}

void ComponentInfo::setField(Registry &registry, Entity entity, std::string_view name, const FieldValue &value) const
{
    this->getFieldEntry(name).set(registry, entity, value);
}

const ComponentInfo::Field *ComponentInfo::findField(std::string_view name) const noexcept
{
    for (const Field &field : this->_fields) {
        if (field.name == name) {
            return &field;
        }
    }
    return nullptr;
}

const ComponentInfo::Field &ComponentInfo::getFieldEntry(std::string_view name) const
{
    const Field *field = this->findField(name);

    if (field == nullptr) {
        throw std::out_of_range("younity::ecs::ComponentInfo: the component " + this->_name + " has no field " + std::string(name));
    }
    return *field;
}

} // namespace younity::ecs

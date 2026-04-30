#ifndef ENTITY_TEMPLATE_TYPES_HPP
#define ENTITY_TEMPLATE_TYPES_HPP

#include <string>
#include <vector>

#include "Character.hpp"

struct EntityTemplateDefinition
{
    std::string id;
    std::string matchPrefix;
    Character::EntityKind entityKind = Character::EntityKind::Unknown;
    Character::MaterialType materialType = Character::MaterialType::None;
    Character::PhysicsState physicsState;
    float health = 1.0f;
    bool hasMatchPrefix = false;
};

struct ParsedEntityTemplateData
{
    std::vector<EntityTemplateDefinition> templates;
};

#endif // ENTITY_TEMPLATE_TYPES_HPP
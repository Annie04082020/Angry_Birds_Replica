#include "EntityTemplateParser.hpp"

#include "JsonParseUtils.hpp"

namespace
{
    Character::EntityKind ParseEntityKind(const std::string &kind)
    {
        if (kind == "Bird")
        {
            return Character::EntityKind::Bird;
        }
        if (kind == "Pig")
        {
            return Character::EntityKind::Pig;
        }
        if (kind == "Slingshot")
        {
            return Character::EntityKind::Slingshot;
        }
        if (kind == "Environment")
        {
            return Character::EntityKind::Environment;
        }
        return Character::EntityKind::Unknown;
    }

    Character::MaterialType ParseMaterialType(const std::string &material)
    {
        if (material == "Flesh")
        {
            return Character::MaterialType::Flesh;
        }
        if (material == "Wood")
        {
            return Character::MaterialType::Wood;
        }
        if (material == "Stone")
        {
            return Character::MaterialType::Stone;
        }
        if (material == "Glass")
        {
            return Character::MaterialType::Glass;
        }
        if (material == "Ice")
        {
            return Character::MaterialType::Ice;
        }
        return Character::MaterialType::None;
    }
}

namespace EntityTemplateParser
{
    ParsedEntityTemplateData Parse(const std::string &jsonStr)
    {
        ParsedEntityTemplateData data;

        std::string templatesContent;
        if (!JsonParseUtils::ExtractArrayContent(jsonStr, "templates", templatesContent))
        {
            return data;
        }

        const auto templateObjects = JsonParseUtils::ExtractObjectContentsFromArray("[" + templatesContent + "]");
        for (const auto &templateJson : templateObjects)
        {
            EntityTemplateDefinition definition;
            definition.id = JsonParseUtils::ExtractString(templateJson, "id");
            if (definition.id.empty())
            {
                continue;
            }

            definition.matchPrefix = JsonParseUtils::ExtractString(templateJson, "matchPrefix");
            definition.hasMatchPrefix = !definition.matchPrefix.empty();

            const std::string kind = JsonParseUtils::ExtractString(templateJson, "kind");
            if (!kind.empty())
            {
                definition.entityKind = ParseEntityKind(kind);
            }

            const std::string material = JsonParseUtils::ExtractString(templateJson, "material");
            if (!material.empty())
            {
                definition.materialType = ParseMaterialType(material);
            }

            if (JsonParseUtils::HasKey(templateJson, "mass"))
            {
                definition.physicsState.mass = JsonParseUtils::ExtractFloat(templateJson, "mass", definition.physicsState.mass);
            }
            if (JsonParseUtils::HasKey(templateJson, "inertia"))
            {
                definition.physicsState.inertia = JsonParseUtils::ExtractFloat(templateJson, "inertia", definition.physicsState.inertia);
            }
            if (JsonParseUtils::HasKey(templateJson, "velocityX"))
            {
                definition.physicsState.velocity.x = JsonParseUtils::ExtractFloat(templateJson, "velocityX", definition.physicsState.velocity.x);
            }
            if (JsonParseUtils::HasKey(templateJson, "velocityY"))
            {
                definition.physicsState.velocity.y = JsonParseUtils::ExtractFloat(templateJson, "velocityY", definition.physicsState.velocity.y);
            }
            if (JsonParseUtils::HasKey(templateJson, "angularVelocity"))
            {
                definition.physicsState.angularVelocity = JsonParseUtils::ExtractFloat(templateJson, "angularVelocity", definition.physicsState.angularVelocity);
            }
            if (JsonParseUtils::HasKey(templateJson, "centerOfMassOffsetX"))
            {
                definition.physicsState.centerOfMassOffset.x = JsonParseUtils::ExtractFloat(templateJson, "centerOfMassOffsetX", definition.physicsState.centerOfMassOffset.x);
            }
            if (JsonParseUtils::HasKey(templateJson, "centerOfMassOffsetY"))
            {
                definition.physicsState.centerOfMassOffset.y = JsonParseUtils::ExtractFloat(templateJson, "centerOfMassOffsetY", definition.physicsState.centerOfMassOffset.y);
            }
            if (JsonParseUtils::HasKey(templateJson, "isStatic"))
            {
                definition.physicsState.isStatic = JsonParseUtils::ExtractBool(templateJson, "isStatic", definition.physicsState.isStatic);
            }
            if (JsonParseUtils::HasKey(templateJson, "health"))
            {
                definition.health = JsonParseUtils::ExtractFloat(templateJson, "health", definition.health);
            }

            data.templates.push_back(definition);
        }

        return data;
    }
} // namespace EntityTemplateParser
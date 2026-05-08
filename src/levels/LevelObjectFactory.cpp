#include "LevelObjectFactory.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "Character.hpp"
#include "EntityTemplateDatabase.hpp"
#include "Resource.hpp"
#include "Util/TransformUtils.hpp"
#include "Util/ViewportUtils.hpp"

#include <cctype>

namespace
{
    bool StartsWith(const std::string &value, const std::string &prefix)
    {
        return value.rfind(prefix, 0) == 0;
    }

    std::string ToLowerCopy(const std::string &value)
    {
        std::string lowered = value;
        for (char &ch : lowered)
        {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        return lowered;
    }

    bool IsEarthKey(const std::string &value)
    {
        return StartsWith(value, "EARTH");
    }

    Character::EntityKind ClassifyEntityKind(const std::string &imageId)
    {
        if (imageId.rfind("BIRD", 0) == 0)
        {
            return Character::EntityKind::Bird;
        }
        if (imageId.rfind("PIG", 0) == 0)
        {
            return Character::EntityKind::Pig;
        }
        if (imageId.rfind("SLINGSHOT", 0) == 0)
        {
            return Character::EntityKind::Slingshot;
        }
        if (imageId.rfind("WOOD", 0) == 0 || imageId.rfind("STONE", 0) == 0 ||
            imageId.rfind("GLASS", 0) == 0 || imageId.rfind("EARTH", 0) == 0)
        {
            return Character::EntityKind::Environment;
        }
        return Character::EntityKind::Unknown;
    }

    Character::MaterialType ClassifyMaterialType(const std::string &imageId)
    {
        if (imageId.rfind("WOOD", 0) == 0)
        {
            return Character::MaterialType::Wood;
        }
        if (imageId.rfind("STONE", 0) == 0)
        {
            return Character::MaterialType::Stone;
        }
        if (imageId.rfind("ICE", 0) == 0)
        {
            return Character::MaterialType::Ice;
        }
        if (imageId.rfind("GLASS", 0) == 0)
        {
            return Character::MaterialType::Glass;
        }
        if (imageId.rfind("EARTH", 0) == 0)
        {
            return Character::MaterialType::Earth;
        }
        if (imageId.rfind("BIRD", 0) == 0 || imageId.rfind("PIG", 0) == 0)
        {
            return Character::MaterialType::Flesh;
        }
        return Character::MaterialType::None;
    }

    Character::ColliderShape ClassifyColliderShape(const LevelObjectDefinition &objectDefinition, const std::string &usedId)
    {
        std::string shapeKey = objectDefinition.collisionShape;
        if (shapeKey.empty())
        {
            shapeKey = objectDefinition.typeStr;
        }
        if (shapeKey.empty())
        {
            shapeKey = usedId;
        }

        const std::string lowered = ToLowerCopy(shapeKey);
        if (lowered.find("triangle_down") != std::string::npos ||
            lowered.find("tri_down") != std::string::npos ||
            lowered.find("triangle-down") != std::string::npos)
        {
            return Character::ColliderShape::TriangleDown;
        }
        if (lowered.find("triangle_left") != std::string::npos ||
            lowered.find("tri_left") != std::string::npos ||
            lowered.find("triangle-left") != std::string::npos)
        {
            return Character::ColliderShape::TriangleLeft;
        }
        if (lowered.find("triangle_right") != std::string::npos ||
            lowered.find("tri_right") != std::string::npos ||
            lowered.find("triangle-right") != std::string::npos)
        {
            return Character::ColliderShape::TriangleRight;
        }
        if (lowered.find("triangle") != std::string::npos ||
            lowered.find("tri_up") != std::string::npos ||
            lowered.find("tri") != std::string::npos)
        {
            return Character::ColliderShape::TriangleUp;
        }

        return Character::ColliderShape::Box;
    }

    void ApplyTemplateDefaults(Character &character, const std::string &imageId)
    {
        if (imageId.rfind("SLINGSHOT", 0) == 0)
        {
            character.SetEntityKind(Character::EntityKind::Slingshot);
            character.SetMaterialType(Character::MaterialType::None);
            character.SetStatic(true);
            return;
        }

        const EntityTemplateDefinition *definition = EntityTemplateDatabase::FindTemplate(imageId);
        if (definition)
        {
            character.SetPhysicsState(definition->physicsState);
            if (definition->entityKind != Character::EntityKind::Unknown)
            {
                character.SetEntityKind(definition->entityKind);
            }
            if (definition->materialType != Character::MaterialType::None)
            {
                character.SetMaterialType(definition->materialType);
            }
            // Apply health from template
            character.SetMaxHealth(definition->health);
            character.SetHealth(definition->health);
            // Apply health from template
            character.SetMaxHealth(definition->health);
            character.SetHealth(definition->health);
        }

        if (character.GetEntityKind() == Character::EntityKind::Unknown)
        {
            character.SetEntityKind(ClassifyEntityKind(imageId));
        }
        if (character.GetMaterialType() == Character::MaterialType::None)
        {
            character.SetMaterialType(ClassifyMaterialType(imageId));
        }
    }

    std::string PrepareResourcePath(const std::string &resourcePath)
    {
        if (resourcePath.empty() || resourcePath.find(':') != std::string::npos || resourcePath[0] == '/')
        {
            return resourcePath;
        }

        return std::string(RESOURCE_DIR) + "/" + resourcePath;
    }

    std::string ResolveResourcePath(const LevelObjectDefinition &objectDefinition,
                                    const ParsedLevelData &levelData,
                                    std::string &usedId)
    {
        const std::string candidateId = !objectDefinition.imageId.empty()
                                            ? objectDefinition.imageId
                                            : (IsEarthKey(objectDefinition.typeStr) ? objectDefinition.typeStr : std::string());

        auto lookup = [&](const std::string &id) -> std::string
        {
            const std::string globalPath = Resource::GetPath(id);
            if (!globalPath.empty())
            {
                return globalPath;
            }

            auto it = levelData.resourceMap.find(id);
            if (it != levelData.resourceMap.end())
            {
                return it->second;
            }
            return std::string();
        };

        std::string resourcePath = lookup(candidateId);
        if (!resourcePath.empty())
        {
            usedId = candidateId;
            return resourcePath;
        }

        for (int s = 1; resourcePath.empty() && s <= 4; ++s)
        {
            if (candidateId.empty())
            {
                break;
            }

            const std::string variantId = candidateId + "_" + std::to_string(s);
            resourcePath = lookup(variantId);
            if (!resourcePath.empty())
            {
                usedId = variantId;
                return resourcePath;
            }
        }

        return std::string();
    }

    std::string NormalizeDamageBaseImageId(const std::string &imageId)
    {
        const std::size_t lastUnderscore = imageId.rfind('_');
        if (lastUnderscore == std::string::npos || lastUnderscore + 1 >= imageId.size())
        {
            return imageId;
        }

        const std::string suffix = imageId.substr(lastUnderscore + 1);
        const bool isNumericSuffix = std::all_of(suffix.begin(), suffix.end(), [](unsigned char ch) {
            return std::isdigit(ch) != 0;
        });
        return isNumericSuffix ? imageId.substr(0, lastUnderscore) : imageId;
    }

    // Count how many damage state variants are available for a given base image ID
    // e.g., "WOOD_205" -> check WOOD_205_1, WOOD_205_2, ... and count them
    int CountAvailableDamageStates(const std::string &baseImageId)
    {
        const std::string normalizedBaseImageId = NormalizeDamageBaseImageId(baseImageId);
        // Check how many _1, _2, _3, ... variants exist in Resource
        int count = 0;
        for (int i = 1; i <= 10; ++i) // Check up to 10 variants
        {
            const std::string variantId = normalizedBaseImageId + "_" + std::to_string(i);
            if (Resource::GetPath(variantId).empty())
            {
                // No more variants found
                break;
            }
            count++; // Found variant _i
        }
        // If no explicit numbered variants found, but the base id maps to a resource,
        // treat that as a single available state.
        if (count == 0 && !Resource::GetPath(normalizedBaseImageId).empty())
            return 1;
        return count;
    }
} // namespace

std::shared_ptr<Character> LevelObjectFactory::CreateCharacter(const LevelObjectDefinition &objectDefinition,
                                                               const ParsedLevelData &levelData,
                                                               float runtimeScale)
{
    if (objectDefinition.imageId.empty())
    {
        std::cerr << "Warning: No imageId/resourceId found for object in level JSON. Skipping object." << std::endl;
        return nullptr;
    }

    std::string usedId;
    const std::string resourcePath = ResolveResourcePath(objectDefinition, levelData, usedId);
    if (resourcePath.empty())
    {
        std::cerr << "Warning: Image ID '" << objectDefinition.imageId << "' (or variants _1.._4) not found in local or global resource map" << std::endl;
        return nullptr;
    }
    if (usedId != objectDefinition.imageId)
    {
        std::cout << "LevelObjectFactory: imageId fallback: '" << objectDefinition.imageId
                  << "' -> using '" << usedId << "'" << std::endl;
    }

    float posX = objectDefinition.posX;
    float posY = objectDefinition.posY;
    float scaleX = objectDefinition.scaleX <= 0.0f ? 1.0f : objectDefinition.scaleX;
    float scaleY = objectDefinition.scaleY <= 0.0f ? 1.0f : objectDefinition.scaleY;

    auto groupIt = levelData.groupAdjustments.find(objectDefinition.groupId);
    if (groupIt != levelData.groupAdjustments.end())
    {
        posX += groupIt->second.offsetX * groupIt->second.scaleMultiplier;
        posY += groupIt->second.offsetY * groupIt->second.scaleMultiplier;
    }

    posX *= runtimeScale;
    posY *= runtimeScale;

    auto character = std::make_shared<Character>(PrepareResourcePath(resourcePath));
    const glm::vec2 textureSize = character->GetSize();
    const glm::vec2 viewportSize = Util::GetViewportSize();
    const float viewportWidth = viewportSize.x;
    const float viewportHeight = viewportSize.y;

    if (objectDefinition.hasSizePercent)
    {
        float uniformScale = 1.0f;
        if ((objectDefinition.sizeBase == "height" || objectDefinition.sizeBase == "h" || objectDefinition.sizeBase == "y") &&
            textureSize.y > 0.0f && viewportHeight > 0.0f)
        {
            uniformScale = (viewportHeight * objectDefinition.sizePercent / 100.0f) / textureSize.y;
        }
        else if (textureSize.x > 0.0f && viewportWidth > 0.0f)
        {
            uniformScale = (viewportWidth * objectDefinition.sizePercent / 100.0f) / textureSize.x;
        }

        scaleX = uniformScale;
        scaleY = uniformScale;
    }
    else
    {
        if (objectDefinition.hasWidthPercent && !objectDefinition.hasHeightPercent && textureSize.x > 0.0f && viewportWidth > 0.0f)
        {
            const float uniformScale = (viewportWidth * objectDefinition.widthPercent / 100.0f) / textureSize.x;
            scaleX = uniformScale;
            scaleY = uniformScale;
        }
        else if (objectDefinition.hasHeightPercent && !objectDefinition.hasWidthPercent && textureSize.y > 0.0f && viewportHeight > 0.0f)
        {
            const float uniformScale = (viewportHeight * objectDefinition.heightPercent / 100.0f) / textureSize.y;
            scaleX = uniformScale;
            scaleY = uniformScale;
        }
        else
        {
            if (objectDefinition.hasWidthPercent)
            {
                if (textureSize.x > 0.0f && viewportWidth > 0.0f)
                {
                    scaleX = (viewportWidth * objectDefinition.widthPercent / 100.0f) / textureSize.x;
                }
            }
            else
            {
                scaleX *= runtimeScale;
            }

            if (objectDefinition.hasHeightPercent)
            {
                if (textureSize.y > 0.0f && viewportHeight > 0.0f)
                {
                    scaleY = (viewportHeight * objectDefinition.heightPercent / 100.0f) / textureSize.y;
                }
            }
            else
            {
                scaleY *= runtimeScale;
            }
        }
    }

    if (groupIt != levelData.groupAdjustments.end())
    {
        if (groupIt->second.scalePosition &&
            groupIt->second.hasScalePivot &&
            groupIt->second.scaleMultiplier != 1.0f)
        {
            const float scaledPivotX = groupIt->second.scalePivotX * runtimeScale;
            const float scaledPivotY = groupIt->second.scalePivotY * runtimeScale;
            float relX = posX - scaledPivotX;
            float relY = posY - scaledPivotY;
            relX *= groupIt->second.scaleMultiplier;
            relY *= groupIt->second.scaleMultiplier;
            posX = scaledPivotX + relX;
            posY = scaledPivotY + relY;
        }

        scaleX *= groupIt->second.scaleMultiplier;
        scaleY *= groupIt->second.scaleMultiplier;
    }

    character->SetPosition(glm::vec2(posX, posY));
    character->SetScale(glm::vec2(scaleX, scaleY));
    character->SetRotation(objectDefinition.rotation);
    character->SetVisible(true);
    character->SetBaseImageId(objectDefinition.imageId); // Store base ID for damage state switching

    // Detect and set the number of available damage state images
    int numDamageStates = CountAvailableDamageStates(objectDefinition.imageId);
    character->SetNumDamageStates(numDamageStates);

    ApplyTemplateDefaults(*character, objectDefinition.imageId);

    const bool isDecor = objectDefinition.typeStr == "DECOR";

    if (isDecor)
    {
        character->SetEntityKind(Character::EntityKind::Unknown);
        character->SetMaterialType(Character::MaterialType::None);
        character->SetStatic(true);
        character->SetSleeping(true);
        character->SetImpactActivated(false);
        character->SetParticipatesInPhysics(false);
        character->SetVelocity({0.0f, 0.0f});
        character->SetAngularVelocity(0.0f);
    }

    if (!isDecor && character->GetEntityKind() == Character::EntityKind::Environment)
    {
        character->SetImpactActivated(false);
        character->SetStatic(true);
        character->SetSleeping(true);
        character->SetVelocity({0.0f, 0.0f});
        character->SetAngularVelocity(0.0f);
    }
    else
    {
        character->SetImpactActivated(true);
    }

    if (IsEarthKey(objectDefinition.typeStr) || StartsWith(objectDefinition.imageId, "EARTH"))
    {
        character->SetEntityKind(Character::EntityKind::Environment);
        character->SetMaterialType(Character::MaterialType::Earth);
    }

    character->SetColliderShape(ClassifyColliderShape(objectDefinition, usedId));

    if (StartsWith(objectDefinition.imageId, "WOOD_tri") || StartsWith(objectDefinition.imageId, "WOOD_tri_empty"))
    {
        character->SetColliderShape(ClassifyColliderShape(objectDefinition, usedId));
    }

    const bool isDecor = objectDefinition.typeStr == "DECOR";

    if (isDecor)
    {
        character->SetEntityKind(Character::EntityKind::Unknown);
        character->SetMaterialType(Character::MaterialType::None);
        character->SetStatic(true);
        character->SetSleeping(true);
        character->SetImpactActivated(false);
        character->SetParticipatesInPhysics(false);
        character->SetVelocity({0.0f, 0.0f});
        character->SetAngularVelocity(0.0f);
    }

    if (!isDecor && character->GetEntityKind() == Character::EntityKind::Environment)
    {
        character->SetSleeping(true);
        character->SetVelocity({0.0f, 0.0f});
        character->SetAngularVelocity(0.0f);
    }
    else
    {
        character->SetImpactActivated(true);
    }

    if (objectDefinition.imageId == "SLINGSHOT_1" || 
        objectDefinition.imageId == "SLINGSHOT_2" ||
        objectDefinition.imageId == "sprite_147" ||
        objectDefinition.imageId == "sprite_154")
    {
        character->SetEntityKind(Character::EntityKind::Slingshot);
        character->SetParticipatesInPhysics(false);
        if (objectDefinition.imageId == "SLINGSHOT_1") character->SetZIndex(1.0f);
        else if (objectDefinition.imageId == "SLINGSHOT_2") character->SetZIndex(-1.0f);
    }

    if (!objectDefinition.imageId.empty())
    {
        if (objectDefinition.imageId.rfind("WOOD", 0) == 0 || objectDefinition.imageId.rfind("STONE", 0) == 0 ||
            objectDefinition.imageId.rfind("EARTH", 0) == 0)
        {
            character->SetZIndex(0.0f);
    
        }
        else if (objectDefinition.imageId.rfind("BIRD", 0) == 0)
        {
            character->SetZIndex(10.0f);
        }
    }

    if (objectDefinition.hasZIndex)
    {
        character->SetZIndex(objectDefinition.zIndex);
    }

    return character;
}

#include "LevelObjectFactory.hpp"

#include <iostream>

#include "Character.hpp"
#include "Resource.hpp"
#include "Util/TransformUtils.hpp"

namespace
{
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

        std::string resourcePath = lookup(objectDefinition.imageId);
        if (!resourcePath.empty())
        {
            usedId = objectDefinition.imageId;
            return resourcePath;
        }

        for (int s = 1; resourcePath.empty() && s <= 4; ++s)
        {
            const std::string variantId = objectDefinition.imageId + "_" + std::to_string(s);
            resourcePath = lookup(variantId);
            if (!resourcePath.empty())
            {
                usedId = variantId;
                return resourcePath;
            }
        }

        return std::string();
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
    const float viewportWidth = viewportSize.x * runtimeScale;
    const float viewportHeight = viewportSize.y * runtimeScale;

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

    if (objectDefinition.imageId == "SLINGSHOT_1")
    {
        character->SetZIndex(1.0f);
    }
    else if (objectDefinition.imageId == "SLINGSHOT_2")
    {
        character->SetZIndex(-1.0f);
    }

    if (!objectDefinition.imageId.empty())
    {
        if (objectDefinition.imageId.rfind("WOOD", 0) == 0 || objectDefinition.imageId.rfind("STONE", 0) == 0)
        {
            character->SetZIndex(50.0f);
        }
    }

    return character;
}
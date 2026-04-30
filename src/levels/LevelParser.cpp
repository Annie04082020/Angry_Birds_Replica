#include "LevelParser.hpp"

#include "JsonParseUtils.hpp"
#include "LayoutParseUtils.hpp"

#include <cmath>
#include <iostream>

#include "config.hpp"

ParsedLevelData LevelParser::Parse(const std::string &jsonStr)
{
    ParsedLevelData data;
    data.levelName = JsonParseUtils::ExtractString(jsonStr, "name");
    data.backgroundImage = JsonParseUtils::ExtractString(jsonStr, "background");
    data.birdCount = JsonParseUtils::ExtractInt(jsonStr, "birds");
    data.resourceMap = LayoutParseUtils::ExtractStringMapField(jsonStr, "resources");
    data.groupAdjustments = LayoutParseUtils::ExtractGroupAdjustments(
        jsonStr,
        LayoutParseUtils::GroupCoordinateSpace::PixelSpace,
        static_cast<float>(WINDOW_WIDTH),
        static_cast<float>(WINDOW_HEIGHT));

    std::string objectsContent;
    if (!JsonParseUtils::ExtractArrayContent(jsonStr, "objects", objectsContent))
    {
        std::cerr << "No objects array found in level JSON" << std::endl;
        return data;
    }

    const auto objectJsonList = JsonParseUtils::ExtractObjectContentsFromArray("[" + objectsContent + "]");
    for (const auto &entityJson : objectJsonList)
    {
        LevelObjectDefinition objectDefinition;

        objectDefinition.typeStr = JsonParseUtils::ExtractString(entityJson, "type");
        objectDefinition.posX = JsonParseUtils::HasKey(entityJson, "xPercent")
                                    ? JsonParseUtils::ExtractFloat(entityJson, "xPercent") * static_cast<float>(WINDOW_WIDTH) / 100.0f
                                    : JsonParseUtils::ExtractFloat(entityJson, "x");
        objectDefinition.posY = JsonParseUtils::HasKey(entityJson, "yPercent")
                                    ? JsonParseUtils::ExtractFloat(entityJson, "yPercent") * static_cast<float>(WINDOW_HEIGHT) / 100.0f
                                    : JsonParseUtils::ExtractFloat(entityJson, "y");
        objectDefinition.scaleX = JsonParseUtils::ExtractFloat(entityJson, "scaleX");
        objectDefinition.scaleY = JsonParseUtils::ExtractFloat(entityJson, "scaleY");
        objectDefinition.rotation = JsonParseUtils::ExtractFloat(entityJson, "rotation");
        objectDefinition.groupId = JsonParseUtils::ExtractString(entityJson, "groupId");

        objectDefinition.hasSizePercent = JsonParseUtils::HasKey(entityJson, "sizePercent") || JsonParseUtils::HasKey(entityJson, "scalePercent");
        objectDefinition.hasWidthPercent = JsonParseUtils::HasKey(entityJson, "widthPercent") || JsonParseUtils::HasKey(entityJson, "scalePercentX");
        objectDefinition.hasHeightPercent = JsonParseUtils::HasKey(entityJson, "heightPercent") || JsonParseUtils::HasKey(entityJson, "scalePercentY");

        objectDefinition.sizeBase = JsonParseUtils::ExtractString(entityJson, "sizeBase");
        if (objectDefinition.sizeBase.empty())
        {
            objectDefinition.sizeBase = JsonParseUtils::ExtractString(entityJson, "scaleBase");
        }
        if (objectDefinition.sizeBase.empty())
        {
            objectDefinition.sizeBase = "height";
        }

        if (objectDefinition.hasSizePercent)
        {
            objectDefinition.sizePercent = JsonParseUtils::HasKey(entityJson, "sizePercent")
                                               ? JsonParseUtils::ExtractFloat(entityJson, "sizePercent")
                                               : JsonParseUtils::ExtractFloat(entityJson, "scalePercent");
        }

        if (objectDefinition.hasWidthPercent)
        {
            objectDefinition.widthPercent = JsonParseUtils::HasKey(entityJson, "widthPercent")
                                                ? JsonParseUtils::ExtractFloat(entityJson, "widthPercent")
                                                : JsonParseUtils::ExtractFloat(entityJson, "scalePercentX");
        }
        if (objectDefinition.hasHeightPercent)
        {
            objectDefinition.heightPercent = JsonParseUtils::HasKey(entityJson, "heightPercent")
                                                 ? JsonParseUtils::ExtractFloat(entityJson, "heightPercent")
                                                 : JsonParseUtils::ExtractFloat(entityJson, "scalePercentY");
        }

        objectDefinition.imageId = JsonParseUtils::ExtractString(entityJson, "imageId");
        if (objectDefinition.imageId.empty())
        {
            objectDefinition.imageId = JsonParseUtils::ExtractString(entityJson, "resourceId");
        }

        data.objects.push_back(objectDefinition);
    }

    return data;
}
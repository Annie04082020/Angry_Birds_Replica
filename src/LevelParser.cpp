#include "LevelParser.hpp"

#include "JsonParseUtils.hpp"

#include <cmath>
#include <iostream>

#include "config.hpp"

namespace
{
    std::unordered_map<std::string, std::string> ExtractResourceMap(const std::string &jsonStr)
    {
        std::unordered_map<std::string, std::string> resourceMap;

        const size_t resourcesStart = jsonStr.find("\"resources\"");
        if (resourcesStart == std::string::npos)
        {
            return resourceMap;
        }

        const size_t objectStart = jsonStr.find('{', resourcesStart);
        if (objectStart == std::string::npos)
        {
            return resourceMap;
        }

        size_t braceCount = 0;
        size_t objectEnd = objectStart;
        for (size_t currentPos = objectStart; currentPos < jsonStr.length(); ++currentPos)
        {
            if (jsonStr[currentPos] == '{')
            {
                ++braceCount;
            }
            else if (jsonStr[currentPos] == '}')
            {
                --braceCount;
                if (braceCount == 0)
                {
                    objectEnd = currentPos;
                    break;
                }
            }
        }

        if (objectEnd <= objectStart)
        {
            return resourceMap;
        }

        const std::string resourcesContent = jsonStr.substr(objectStart, objectEnd - objectStart + 1);
        return JsonParseUtils::ExtractStringMapFromObject(resourcesContent);
    }

    std::unordered_map<std::string, GroupAdjustment> ExtractGroupAdjustments(const std::string &jsonStr)
    {
        std::unordered_map<std::string, GroupAdjustment> groups;

        const size_t groupsStart = jsonStr.find("\"groups\"");
        if (groupsStart == std::string::npos)
        {
            return groups;
        }

        const size_t arrayStart = jsonStr.find('[', groupsStart);
        if (arrayStart == std::string::npos)
        {
            return groups;
        }

        size_t arrayEnd = arrayStart;
        int arrayDepth = 0;
        for (size_t i = arrayStart; i < jsonStr.length(); ++i)
        {
            if (jsonStr[i] == '[')
            {
                ++arrayDepth;
            }
            else if (jsonStr[i] == ']')
            {
                --arrayDepth;
                if (arrayDepth == 0)
                {
                    arrayEnd = i;
                    break;
                }
            }
        }

        if (arrayEnd <= arrayStart)
        {
            return groups;
        }

        const std::string groupsContent = jsonStr.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

        size_t objectStart = 0;
        int braceCount = 0;
        for (size_t i = 0; i < groupsContent.length(); ++i)
        {
            if (groupsContent[i] == '{')
            {
                if (braceCount == 0)
                {
                    objectStart = i;
                }
                ++braceCount;
            }
            else if (groupsContent[i] == '}')
            {
                --braceCount;
                if (braceCount == 0)
                {
                    const std::string groupJson = groupsContent.substr(objectStart + 1, i - objectStart - 1);
                    const std::string id = JsonParseUtils::ExtractString(groupJson, "id");
                    if (id.empty())
                    {
                        continue;
                    }

                    GroupAdjustment adjustment;

                    if (JsonParseUtils::HasKey(groupJson, "scaleMultiplier"))
                    {
                        adjustment.scaleMultiplier = JsonParseUtils::ExtractFloat(groupJson, "scaleMultiplier");
                    }
                    else if (JsonParseUtils::HasKey(groupJson, "sizeMultiplier"))
                    {
                        adjustment.scaleMultiplier = JsonParseUtils::ExtractFloat(groupJson, "sizeMultiplier");
                    }
                    else if (JsonParseUtils::HasKey(groupJson, "scale"))
                    {
                        adjustment.scaleMultiplier = JsonParseUtils::ExtractFloat(groupJson, "scale");
                    }

                    if (adjustment.scaleMultiplier <= 0.0f)
                    {
                        adjustment.scaleMultiplier = 1.0f;
                    }

                    if (JsonParseUtils::HasKey(groupJson, "offsetXPercent"))
                    {
                        adjustment.offsetX = JsonParseUtils::ExtractFloat(groupJson, "offsetXPercent") * static_cast<float>(WINDOW_WIDTH) / 100.0f;
                    }
                    else if (JsonParseUtils::HasKey(groupJson, "offsetX"))
                    {
                        adjustment.offsetX = JsonParseUtils::ExtractFloat(groupJson, "offsetX");
                    }
                    else if (JsonParseUtils::HasKey(groupJson, "moveX"))
                    {
                        adjustment.offsetX = JsonParseUtils::ExtractFloat(groupJson, "moveX");
                    }

                    if (JsonParseUtils::HasKey(groupJson, "offsetYPercent"))
                    {
                        adjustment.offsetY = JsonParseUtils::ExtractFloat(groupJson, "offsetYPercent") * static_cast<float>(WINDOW_HEIGHT) / 100.0f;
                    }
                    else if (JsonParseUtils::HasKey(groupJson, "offsetY"))
                    {
                        adjustment.offsetY = JsonParseUtils::ExtractFloat(groupJson, "offsetY");
                    }
                    else if (JsonParseUtils::HasKey(groupJson, "moveY"))
                    {
                        adjustment.offsetY = JsonParseUtils::ExtractFloat(groupJson, "moveY");
                    }

                    if (JsonParseUtils::HasKey(groupJson, "scalePivotXPercent") && JsonParseUtils::HasKey(groupJson, "scalePivotYPercent"))
                    {
                        adjustment.scalePivotX = JsonParseUtils::ExtractFloat(groupJson, "scalePivotXPercent") * static_cast<float>(WINDOW_WIDTH) / 100.0f;
                        adjustment.scalePivotY = JsonParseUtils::ExtractFloat(groupJson, "scalePivotYPercent") * static_cast<float>(WINDOW_HEIGHT) / 100.0f;
                        adjustment.hasScalePivot = true;
                    }
                    else if (JsonParseUtils::HasKey(groupJson, "scalePivotX") && JsonParseUtils::HasKey(groupJson, "scalePivotY"))
                    {
                        adjustment.scalePivotX = JsonParseUtils::ExtractFloat(groupJson, "scalePivotX");
                        adjustment.scalePivotY = JsonParseUtils::ExtractFloat(groupJson, "scalePivotY");
                        adjustment.hasScalePivot = true;
                    }

                    if (JsonParseUtils::HasKey(groupJson, "scalePosition"))
                    {
                        adjustment.scalePosition = JsonParseUtils::ExtractFloat(groupJson, "scalePosition") != 0.0f;
                    }

                    groups[id] = adjustment;
                }
            }
        }

        return groups;
    }
} // namespace

ParsedLevelData LevelParser::Parse(const std::string &jsonStr)
{
    ParsedLevelData data;
    data.levelName = JsonParseUtils::ExtractString(jsonStr, "name");
    data.backgroundImage = JsonParseUtils::ExtractString(jsonStr, "background");
    data.birdCount = JsonParseUtils::ExtractInt(jsonStr, "birds");
    data.resourceMap = ExtractResourceMap(jsonStr);
    data.groupAdjustments = ExtractGroupAdjustments(jsonStr);

    const size_t objectsStart = jsonStr.find("\"objects\"");
    if (objectsStart == std::string::npos)
    {
        std::cerr << "No objects array found in level JSON" << std::endl;
        return data;
    }

    const size_t arrayStart = jsonStr.find('[', objectsStart);
    const size_t arrayEnd = jsonStr.find(']', arrayStart);
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos)
    {
        std::cerr << "Invalid objects array format" << std::endl;
        return data;
    }

    const std::string objectsContent = jsonStr.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    size_t objectStart = 0;
    int braceCount = 0;
    for (size_t currentPos = 0; currentPos < objectsContent.length(); ++currentPos)
    {
        if (objectsContent[currentPos] == '{')
        {
            if (braceCount == 0)
            {
                objectStart = currentPos;
            }
            ++braceCount;
        }
        else if (objectsContent[currentPos] == '}')
        {
            --braceCount;
            if (braceCount == 0)
            {
                const std::string entityJson = objectsContent.substr(objectStart + 1, currentPos - objectStart - 1);
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
        }
    }

    return data;
}
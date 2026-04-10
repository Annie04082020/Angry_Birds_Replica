#include "LevelParser.hpp"

#include <cmath>
#include <cctype>
#include <iostream>

#include "config.hpp"

namespace
{
    std::string ExtractJsonString(const std::string &json, const std::string &key)
    {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos)
            return "";

        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
            return "";

        size_t openQuote = json.find('"', colonPos);
        if (openQuote == std::string::npos)
            return "";

        size_t closeQuote = json.find('"', openQuote + 1);
        if (closeQuote == std::string::npos)
            return "";

        return json.substr(openQuote + 1, closeQuote - openQuote - 1);
    }

    int ExtractJsonInt(const std::string &json, const std::string &key)
    {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos)
            return 0;

        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
            return 0;

        size_t numStart = colonPos + 1;
        while (numStart < json.length() && (json[numStart] == ' ' || json[numStart] == '\t'))
        {
            numStart++;
        }

        size_t numEnd = numStart;
        while (numEnd < json.length() && (std::isdigit(json[numEnd]) || json[numEnd] == '-'))
        {
            numEnd++;
        }

        if (numStart < numEnd)
        {
            return std::stoi(json.substr(numStart, numEnd - numStart));
        }
        return 0;
    }

    float ExtractJsonFloat(const std::string &json, const std::string &key)
    {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos)
            return 0.0f;

        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
            return 0.0f;

        size_t numStart = colonPos + 1;
        while (numStart < json.length() && (json[numStart] == ' ' || json[numStart] == '\t'))
        {
            numStart++;
        }

        size_t numEnd = numStart;
        while (numEnd < json.length() && (std::isdigit(json[numEnd]) || json[numEnd] == '.' || json[numEnd] == '-'))
        {
            numEnd++;
        }

        if (numStart < numEnd)
        {
            return std::stof(json.substr(numStart, numEnd - numStart));
        }
        return 0.0f;
    }

    bool HasJsonKey(const std::string &json, const std::string &key)
    {
        return json.find("\"" + key + "\"") != std::string::npos;
    }

    std::unordered_map<std::string, std::string> ExtractJsonObject(const std::string &json)
    {
        std::unordered_map<std::string, std::string> result;

        size_t objectStart = json.find('{');
        if (objectStart == std::string::npos)
            return result;

        size_t objectEnd = json.rfind('}');
        if (objectEnd == std::string::npos || objectEnd <= objectStart)
            return result;

        std::string content = json.substr(objectStart + 1, objectEnd - objectStart - 1);

        size_t pos = 0;
        while (pos < content.length())
        {
            size_t keyStart = content.find('"', pos);
            if (keyStart == std::string::npos)
                break;

            size_t keyEnd = content.find('"', keyStart + 1);
            if (keyEnd == std::string::npos)
                break;

            std::string key = content.substr(keyStart + 1, keyEnd - keyStart - 1);

            size_t colonPos = content.find(':', keyEnd);
            if (colonPos == std::string::npos)
                break;

            size_t valueStart = content.find('"', colonPos);
            if (valueStart == std::string::npos)
                break;

            size_t valueEnd = content.find('"', valueStart + 1);
            if (valueEnd == std::string::npos)
                break;

            result[key] = content.substr(valueStart + 1, valueEnd - valueStart - 1);
            pos = valueEnd + 1;
        }

        return result;
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
                    const std::string id = ExtractJsonString(groupJson, "id");
                    if (id.empty())
                    {
                        continue;
                    }

                    GroupAdjustment adjustment;

                    if (HasJsonKey(groupJson, "scaleMultiplier"))
                    {
                        adjustment.scaleMultiplier = ExtractJsonFloat(groupJson, "scaleMultiplier");
                    }
                    else if (HasJsonKey(groupJson, "sizeMultiplier"))
                    {
                        adjustment.scaleMultiplier = ExtractJsonFloat(groupJson, "sizeMultiplier");
                    }
                    else if (HasJsonKey(groupJson, "scale"))
                    {
                        adjustment.scaleMultiplier = ExtractJsonFloat(groupJson, "scale");
                    }

                    if (adjustment.scaleMultiplier <= 0.0f)
                    {
                        adjustment.scaleMultiplier = 1.0f;
                    }

                    if (HasJsonKey(groupJson, "offsetXPercent"))
                    {
                        adjustment.offsetX = ExtractJsonFloat(groupJson, "offsetXPercent") * static_cast<float>(WINDOW_WIDTH) / 100.0f;
                    }
                    else if (HasJsonKey(groupJson, "offsetX"))
                    {
                        adjustment.offsetX = ExtractJsonFloat(groupJson, "offsetX");
                    }
                    else if (HasJsonKey(groupJson, "moveX"))
                    {
                        adjustment.offsetX = ExtractJsonFloat(groupJson, "moveX");
                    }

                    if (HasJsonKey(groupJson, "offsetYPercent"))
                    {
                        adjustment.offsetY = ExtractJsonFloat(groupJson, "offsetYPercent") * static_cast<float>(WINDOW_HEIGHT) / 100.0f;
                    }
                    else if (HasJsonKey(groupJson, "offsetY"))
                    {
                        adjustment.offsetY = ExtractJsonFloat(groupJson, "offsetY");
                    }
                    else if (HasJsonKey(groupJson, "moveY"))
                    {
                        adjustment.offsetY = ExtractJsonFloat(groupJson, "moveY");
                    }

                    if (HasJsonKey(groupJson, "scalePivotXPercent") && HasJsonKey(groupJson, "scalePivotYPercent"))
                    {
                        adjustment.scalePivotX = ExtractJsonFloat(groupJson, "scalePivotXPercent") * static_cast<float>(WINDOW_WIDTH) / 100.0f;
                        adjustment.scalePivotY = ExtractJsonFloat(groupJson, "scalePivotYPercent") * static_cast<float>(WINDOW_HEIGHT) / 100.0f;
                        adjustment.hasScalePivot = true;
                    }
                    else if (HasJsonKey(groupJson, "scalePivotX") && HasJsonKey(groupJson, "scalePivotY"))
                    {
                        adjustment.scalePivotX = ExtractJsonFloat(groupJson, "scalePivotX");
                        adjustment.scalePivotY = ExtractJsonFloat(groupJson, "scalePivotY");
                        adjustment.hasScalePivot = true;
                    }

                    if (HasJsonKey(groupJson, "scalePosition"))
                    {
                        adjustment.scalePosition = ExtractJsonFloat(groupJson, "scalePosition") != 0.0f;
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
    data.levelName = ExtractJsonString(jsonStr, "name");
    data.backgroundImage = ExtractJsonString(jsonStr, "background");
    data.birdCount = ExtractJsonInt(jsonStr, "birds");
    data.resourceMap = ExtractJsonObject(jsonStr);
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

                objectDefinition.typeStr = ExtractJsonString(entityJson, "type");
                objectDefinition.posX = HasJsonKey(entityJson, "xPercent")
                                            ? ExtractJsonFloat(entityJson, "xPercent") * static_cast<float>(WINDOW_WIDTH) / 100.0f
                                            : ExtractJsonFloat(entityJson, "x");
                objectDefinition.posY = HasJsonKey(entityJson, "yPercent")
                                            ? ExtractJsonFloat(entityJson, "yPercent") * static_cast<float>(WINDOW_HEIGHT) / 100.0f
                                            : ExtractJsonFloat(entityJson, "y");
                objectDefinition.scaleX = ExtractJsonFloat(entityJson, "scaleX");
                objectDefinition.scaleY = ExtractJsonFloat(entityJson, "scaleY");
                objectDefinition.rotation = ExtractJsonFloat(entityJson, "rotation");
                objectDefinition.groupId = ExtractJsonString(entityJson, "groupId");

                objectDefinition.hasSizePercent = HasJsonKey(entityJson, "sizePercent") || HasJsonKey(entityJson, "scalePercent");
                objectDefinition.hasWidthPercent = HasJsonKey(entityJson, "widthPercent") || HasJsonKey(entityJson, "scalePercentX");
                objectDefinition.hasHeightPercent = HasJsonKey(entityJson, "heightPercent") || HasJsonKey(entityJson, "scalePercentY");

                objectDefinition.sizeBase = ExtractJsonString(entityJson, "sizeBase");
                if (objectDefinition.sizeBase.empty())
                {
                    objectDefinition.sizeBase = ExtractJsonString(entityJson, "scaleBase");
                }
                if (objectDefinition.sizeBase.empty())
                {
                    objectDefinition.sizeBase = "height";
                }

                if (objectDefinition.hasSizePercent)
                {
                    objectDefinition.sizePercent = HasJsonKey(entityJson, "sizePercent")
                                                       ? ExtractJsonFloat(entityJson, "sizePercent")
                                                       : ExtractJsonFloat(entityJson, "scalePercent");
                }

                if (objectDefinition.hasWidthPercent)
                {
                    objectDefinition.widthPercent = HasJsonKey(entityJson, "widthPercent")
                                                        ? ExtractJsonFloat(entityJson, "widthPercent")
                                                        : ExtractJsonFloat(entityJson, "scalePercentX");
                }
                if (objectDefinition.hasHeightPercent)
                {
                    objectDefinition.heightPercent = HasJsonKey(entityJson, "heightPercent")
                                                         ? ExtractJsonFloat(entityJson, "heightPercent")
                                                         : ExtractJsonFloat(entityJson, "scalePercentY");
                }

                objectDefinition.imageId = ExtractJsonString(entityJson, "imageId");
                if (objectDefinition.imageId.empty())
                {
                    objectDefinition.imageId = ExtractJsonString(entityJson, "resourceId");
                }

                data.objects.push_back(objectDefinition);
            }
        }
    }

    return data;
}
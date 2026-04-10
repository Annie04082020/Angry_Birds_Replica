#include "LevelManager.hpp"
#include "Character.hpp"
#include "Resource.hpp"
#include "Util/LoadTextFile.hpp"
#include "Util/TransformUtils.hpp"
#include "SDL.h"
#include "config.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
// Forward-declare Resource::GetPath in case header is not visible to compiler
namespace Resource
{
    std::string GetPath(const std::string &id);
}

namespace
{
    struct GroupAdjustment
    {
        float scaleMultiplier = 1.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float scalePivotX = 0.0f;
        float scalePivotY = 0.0f;
        bool hasScalePivot = false;
        bool scalePosition = true;
    };

    // Simple JSON parsing helpers
    [[maybe_unused]] std::string TrimWhitespace(const std::string &str)
    {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    std::string ExtractJsonString(const std::string &json, const std::string &key)
    {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos)
            return "";

        // Find the colon after the key
        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
            return "";

        // Find the opening quote
        size_t openQuote = json.find('\"', colonPos);
        if (openQuote == std::string::npos)
            return "";

        // Find the closing quote
        size_t closeQuote = json.find('\"', openQuote + 1);
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

        // Extract the number after the colon
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

        // Extract the number after the colon
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

    std::string PrepareResourcePath(const std::string &resourcePath)
    {
        // If path is empty or absolute, return as-is
        if (resourcePath.empty() || resourcePath.find(':') != std::string::npos || resourcePath[0] == '/')
        {
            return resourcePath;
        }

        // This is a relative path, prepend RESOURCE_DIR
        std::string prefix = RESOURCE_DIR;
        return prefix + "/" + resourcePath;
    }

    // Extract key-value pairs from a JSON object string
    std::unordered_map<std::string, std::string> ExtractJsonObject(const std::string &json)
    {
        std::unordered_map<std::string, std::string> result;

        // Find the opening brace
        size_t objectStart = json.find('{');
        if (objectStart == std::string::npos)
            return result;

        size_t objectEnd = json.rfind('}');
        if (objectEnd == std::string::npos || objectEnd <= objectStart)
            return result;

        std::string content = json.substr(objectStart + 1, objectEnd - objectStart - 1);

        // Parse key-value pairs
        size_t pos = 0;
        while (pos < content.length())
        {
            // Find the key
            size_t keyStart = content.find('\"', pos);
            if (keyStart == std::string::npos)
                break;

            size_t keyEnd = content.find('\"', keyStart + 1);
            if (keyEnd == std::string::npos)
                break;

            std::string key = content.substr(keyStart + 1, keyEnd - keyStart - 1);

            // Find the colon
            size_t colonPos = content.find(':', keyEnd);
            if (colonPos == std::string::npos)
                break;

            // Find the value
            size_t valueStart = content.find('\"', colonPos);
            if (valueStart == std::string::npos)
                break;

            size_t valueEnd = content.find('\"', valueStart + 1);
            if (valueEnd == std::string::npos)
                break;

            std::string value = content.substr(valueStart + 1, valueEnd - valueStart - 1);
            result[key] = value;

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

                    // Parse scale pivot point (anchor for scaling)
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

    float GetRuntimeLevelScale()
    {
        static bool initialized = false;
        static float cachedScale = 1.0f;

        if (initialized)
        {
            return cachedScale;
        }
        initialized = true;

        const glm::vec2 viewportSize = Util::GetViewportSize();
        const int windowW = static_cast<int>(viewportSize.x);
        const int windowH = static_cast<int>(viewportSize.y);
        int drawableW = windowW;
        int drawableH = windowH;

        const int effectiveW = drawableW > 0 ? drawableW : windowW;
        const int effectiveH = drawableH > 0 ? drawableH : windowH;

        if (effectiveW <= 0 || effectiveH <= 0)
        {
            std::cerr << "LevelManager: failed to query drawable size, using scale=1.0" << std::endl;
            return cachedScale;
        }

        const float scaleX = static_cast<float>(effectiveW) / static_cast<float>(WINDOW_WIDTH);
        const float scaleY = static_cast<float>(effectiveH) / static_cast<float>(WINDOW_HEIGHT);

        // Use uniform scale to preserve aspect and avoid stretching the layout.
        cachedScale = std::min(scaleX, scaleY);
        if (cachedScale <= 0.0f)
        {
            cachedScale = 1.0f;
        }

        std::cout << "LevelManager: runtime window=" << windowW << "x" << windowH
                  << ", drawable=" << effectiveW << "x" << effectiveH
                  << ", levelScale=" << cachedScale << std::endl;
        return cachedScale;
    }
}

bool LevelManager::LoadLevel(const std::string &levelPath)
{
    std::string fileContent = Util::LoadTextFile(levelPath);
    if (fileContent.empty())
    {
        std::cerr << "Failed to load level file: " << levelPath << std::endl;
        return false;
    }

    Clear();
    return ParseLevelJson(fileContent);
}

std::unordered_map<std::string, std::string> LevelManager::ExtractResourceMap(const std::string &jsonStr)
{
    std::unordered_map<std::string, std::string> resourceMap;

    // Find the resources object
    size_t resourcesStart = jsonStr.find("\"resources\"");
    if (resourcesStart == std::string::npos)
    {
        return resourceMap; // resources block is optional in id-only mode
    }

    // Find the opening brace after "resources"
    size_t objectStart = jsonStr.find('{', resourcesStart);
    if (objectStart == std::string::npos)
        return resourceMap;

    // Find the closing brace for resources object
    size_t braceCount = 0;
    size_t currentPos = objectStart;
    size_t objectEnd = objectStart;

    while (currentPos < jsonStr.length())
    {
        if (jsonStr[currentPos] == '{')
        {
            braceCount++;
        }
        else if (jsonStr[currentPos] == '}')
        {
            braceCount--;
            if (braceCount == 0)
            {
                objectEnd = currentPos;
                break;
            }
        }
        currentPos++;
    }

    // Extract the resources content
    std::string resourcesContent = jsonStr.substr(objectStart, objectEnd - objectStart + 1);

    // Parse key-value pairs from the resources object
    resourceMap = ExtractJsonObject(resourcesContent);

    std::cout << "Loaded " << resourceMap.size() << " resource definitions" << std::endl;
    return resourceMap;
}

bool LevelManager::ParseLevelJson(const std::string &jsonStr)
{
    const float runtimeScale = GetRuntimeLevelScale();
    const auto groupAdjustments = ExtractGroupAdjustments(jsonStr);

    // Extract level metadata
    m_levelName = ExtractJsonString(jsonStr, "name");
    m_backgroundImage = ExtractJsonString(jsonStr, "background");
    m_birdCount = ExtractJsonInt(jsonStr, "birds");

    // Keep compatibility: still parse local map, but object image paths should
    // primarily come from global Resource ids.
    m_resourceMap = ExtractResourceMap(jsonStr);

    // Find the objects array (changed from "entities")
    size_t objectsStart = jsonStr.find("\"objects\"");
    if (objectsStart == std::string::npos)
    {
        std::cerr << "No objects array found in level JSON" << std::endl;
        return false;
    }

    // Find the array opening bracket [
    size_t arrayStart = jsonStr.find('[', objectsStart);
    size_t arrayEnd = jsonStr.find(']', arrayStart);

    if (arrayStart == std::string::npos || arrayEnd == std::string::npos)
    {
        std::cerr << "Invalid objects array format" << std::endl;
        return false;
    }

    std::string objectsContent = jsonStr.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    // Parse each object (separated by commas)
    size_t objectStart = 0;
    int braceCount = 0;
    size_t currentPos = 0;

    while (currentPos < objectsContent.length())
    {
        if (objectsContent[currentPos] == '{')
        {
            if (braceCount == 0)
            {
                objectStart = currentPos;
            }
            braceCount++;
        }
        else if (objectsContent[currentPos] == '}')
        {
            braceCount--;
            if (braceCount == 0)
            {
                std::string entityJson = objectsContent.substr(objectStart + 1, currentPos - objectStart - 1);

                // Parse object properties
                std::string typeStr = ExtractJsonString(entityJson, "type");
                float posX = 0.0f;
                float posY = 0.0f;
                if (HasJsonKey(entityJson, "xPercent"))
                {
                    posX = ExtractJsonFloat(entityJson, "xPercent") * static_cast<float>(WINDOW_WIDTH) / 100.0f;
                }
                else
                {
                    posX = ExtractJsonFloat(entityJson, "x");
                }

                if (HasJsonKey(entityJson, "yPercent"))
                {
                    posY = ExtractJsonFloat(entityJson, "yPercent") * static_cast<float>(WINDOW_HEIGHT) / 100.0f;
                }
                else
                {
                    posY = ExtractJsonFloat(entityJson, "y");
                }
                float scaleX = ExtractJsonFloat(entityJson, "scaleX");
                float scaleY = ExtractJsonFloat(entityJson, "scaleY");
                float rotation = ExtractJsonFloat(entityJson, "rotation");
                const std::string groupId = ExtractJsonString(entityJson, "groupId");
                const bool hasSizePercent = HasJsonKey(entityJson, "sizePercent") ||
                                            HasJsonKey(entityJson, "scalePercent");
                const bool hasWidthPercent = HasJsonKey(entityJson, "widthPercent") ||
                                             HasJsonKey(entityJson, "scalePercentX");
                const bool hasHeightPercent = HasJsonKey(entityJson, "heightPercent") ||
                                              HasJsonKey(entityJson, "scalePercentY");
                float sizePercent = 0.0f;
                float widthPercent = 0.0f;
                float heightPercent = 0.0f;
                std::string sizeBase = ExtractJsonString(entityJson, "sizeBase");

                if (sizeBase.empty())
                {
                    sizeBase = ExtractJsonString(entityJson, "scaleBase");
                }
                if (sizeBase.empty())
                {
                    sizeBase = "height";
                }

                if (hasSizePercent)
                {
                    sizePercent = HasJsonKey(entityJson, "sizePercent")
                                      ? ExtractJsonFloat(entityJson, "sizePercent")
                                      : ExtractJsonFloat(entityJson, "scalePercent");
                }

                if (hasWidthPercent)
                {
                    widthPercent = HasJsonKey(entityJson, "widthPercent")
                                       ? ExtractJsonFloat(entityJson, "widthPercent")
                                       : ExtractJsonFloat(entityJson, "scalePercentX");
                }
                if (hasHeightPercent)
                {
                    heightPercent = HasJsonKey(entityJson, "heightPercent")
                                        ? ExtractJsonFloat(entityJson, "heightPercent")
                                        : ExtractJsonFloat(entityJson, "scalePercentY");
                }

                // Get the resource ID from the object. Support both "imageId" and legacy "resourceId" keys.
                std::string imageId = ExtractJsonString(entityJson, "imageId");
                if (imageId.empty())
                {
                    imageId = ExtractJsonString(entityJson, "resourceId");
                }

                // Look up the actual resource path.
                // Try exact id first (local -> global), then fallback id_1..id_4 for staged sprites.
                std::string resourcePath = "";
                std::string usedId;
                if (!imageId.empty())
                {
                    auto lookup = [&](const std::string &id) -> std::string
                    {
                        // Preferred source: global Resource table
                        auto gp = Resource::GetPath(id);
                        if (!gp.empty())
                        {
                            return gp;
                        }

                        // Backward compatibility: local map in level JSON
                        auto it = m_resourceMap.find(id);
                        if (it != m_resourceMap.end())
                        {
                            return it->second;
                        }
                        return std::string();
                    };

                    resourcePath = lookup(imageId);
                    if (!resourcePath.empty())
                    {
                        usedId = imageId;
                    }

                    for (int s = 1; resourcePath.empty() && s <= 4; ++s)
                    {
                        const std::string variantId = imageId + "_" + std::to_string(s);
                        resourcePath = lookup(variantId);
                        if (!resourcePath.empty())
                        {
                            usedId = variantId;
                            break;
                        }
                    }

                    if (resourcePath.empty())
                    {
                        std::cerr << "Warning: Image ID '" << imageId << "' (or variants _1.._4) not found in local or global resource map" << std::endl;
                        continue; // Skip this object if resource not found
                    }
                    if (usedId != imageId)
                    {
                        std::cout << "LevelManager: imageId fallback: '" << imageId << "' -> using '" << usedId << "'" << std::endl;
                    }
                }
                else
                {
                    std::cerr << "Warning: No imageId/resourceId found for object in level JSON. Skipping object." << std::endl;
                    continue;
                }

                // Validate scale (default to 1.0 if not set)
                if (scaleX <= 0.0f)
                    scaleX = 1.0f;
                if (scaleY <= 0.0f)
                    scaleY = 1.0f;

                auto groupIt = groupAdjustments.find(groupId);
                if (groupIt != groupAdjustments.end())
                {
                    // Apply offset scaled by scaleMultiplier so spacing scales with size
                    posX += groupIt->second.offsetX * groupIt->second.scaleMultiplier;
                    posY += groupIt->second.offsetY * groupIt->second.scaleMultiplier;
                }

                // Apply runtime scaling so level appears proportionally similar
                // across machines with different display resolutions.
                posX *= runtimeScale;
                posY *= runtimeScale;

                // Prepare resource path
                std::string fullPath = PrepareResourcePath(resourcePath);

                // Create character directly
                auto character = std::make_shared<Character>(fullPath);
                const glm::vec2 textureSize = character->GetSize();
                const glm::vec2 viewportSize = Util::GetViewportSize();
                const float viewportWidth = viewportSize.x * runtimeScale;
                const float viewportHeight = viewportSize.y * runtimeScale;

                if (hasSizePercent)
                {
                    float uniformScale = 1.0f;
                    if ((sizeBase == "height" || sizeBase == "h" || sizeBase == "y") &&
                        textureSize.y > 0.0f && viewportHeight > 0.0f)
                    {
                        uniformScale = (viewportHeight * sizePercent / 100.0f) / textureSize.y;
                    }
                    else if (textureSize.x > 0.0f && viewportWidth > 0.0f)
                    {
                        uniformScale = (viewportWidth * sizePercent / 100.0f) / textureSize.x;
                    }

                    scaleX = uniformScale;
                    scaleY = uniformScale;
                }
                else
                {
                    // If only one axis percent is provided, use uniform scale based on that axis.
                    if (hasWidthPercent && !hasHeightPercent && textureSize.x > 0.0f && viewportWidth > 0.0f)
                    {
                        const float uniformScale = (viewportWidth * widthPercent / 100.0f) / textureSize.x;
                        scaleX = uniformScale;
                        scaleY = uniformScale;
                    }
                    else if (hasHeightPercent && !hasWidthPercent && textureSize.y > 0.0f && viewportHeight > 0.0f)
                    {
                        const float uniformScale = (viewportHeight * heightPercent / 100.0f) / textureSize.y;
                        scaleX = uniformScale;
                        scaleY = uniformScale;
                    }
                    else
                    {
                        if (hasWidthPercent)
                        {
                            if (textureSize.x > 0.0f && viewportWidth > 0.0f)
                            {
                                scaleX = (viewportWidth * widthPercent / 100.0f) / textureSize.x;
                            }
                        }
                        else
                        {
                            scaleX *= runtimeScale;
                        }

                        if (hasHeightPercent)
                        {
                            if (textureSize.y > 0.0f && viewportHeight > 0.0f)
                            {
                                scaleY = (viewportHeight * heightPercent / 100.0f) / textureSize.y;
                            }
                        }
                        else
                        {
                            scaleY *= runtimeScale;
                        }
                    }
                }

                if (groupIt != groupAdjustments.end())
                {
                    // If group has a scale pivot point, scale position relative to pivot
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
                character->SetRotation(rotation);
                character->SetVisible(true);
                // If this is a wood/stone asset, ensure it is drawn above background
                if (!imageId.empty())
                {
                    if (imageId.rfind("WOOD", 0) == 0 || imageId.rfind("STONE", 0) == 0)
                    {
                        character->SetZIndex(50.0f);
                    }
                }

                m_gameObjects.push_back(character);
            }
        }
        currentPos++;
    }

    std::cout << "Loaded level: " << m_levelName << std::endl;
    std::cout << "Game objects loaded: " << m_gameObjects.size() << std::endl;
    std::cout << "Available birds: " << m_birdCount << std::endl;

    return true;
}

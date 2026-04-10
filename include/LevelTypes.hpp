#ifndef LEVEL_TYPES_HPP
#define LEVEL_TYPES_HPP

#include <string>
#include <unordered_map>
#include <vector>

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

struct LevelObjectDefinition
{
    std::string typeStr;
    std::string imageId;
    std::string groupId;
    std::string sizeBase;
    std::string resourcePath;
    float posX = 0.0f;
    float posY = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float rotation = 0.0f;
    float sizePercent = 0.0f;
    float widthPercent = 0.0f;
    float heightPercent = 0.0f;
    bool hasSizePercent = false;
    bool hasWidthPercent = false;
    bool hasHeightPercent = false;
};

struct ParsedLevelData
{
    std::string levelName;
    std::string backgroundImage;
    int birdCount = 0;
    std::unordered_map<std::string, std::string> resourceMap;
    std::unordered_map<std::string, GroupAdjustment> groupAdjustments;
    std::vector<LevelObjectDefinition> objects;
};

#endif // LEVEL_TYPES_HPP
#include "LayoutParseUtils.hpp"

#include "JsonParseUtils.hpp"

namespace LayoutParseUtils
{
    std::unordered_map<std::string, std::string> ExtractStringMapField(
        const std::string &json,
        const std::string &fieldKey)
    {
        const std::string key = "\"" + fieldKey + "\"";
        const size_t fieldPos = json.find(key);
        if (fieldPos == std::string::npos)
        {
            return {};
        }

        const size_t objectStart = json.find('{', fieldPos + key.length());
        if (objectStart == std::string::npos)
        {
            return {};
        }

        int braceCount = 0;
        size_t objectEnd = objectStart;
        for (size_t i = objectStart; i < json.length(); ++i)
        {
            if (json[i] == '{')
            {
                ++braceCount;
            }
            else if (json[i] == '}')
            {
                --braceCount;
                if (braceCount == 0)
                {
                    objectEnd = i;
                    break;
                }
            }
        }

        if (objectEnd <= objectStart)
        {
            return {};
        }

        const std::string objectContent =
            json.substr(objectStart, objectEnd - objectStart + 1);
        return JsonParseUtils::ExtractStringMapFromObject(objectContent);
    }

    std::unordered_map<std::string, GroupAdjustment> ExtractGroupAdjustments(
        const std::string &json,
        GroupCoordinateSpace coordinateSpace,
        float percentBaseWidth,
        float percentBaseHeight)
    {
        std::unordered_map<std::string, GroupAdjustment> groups;

        const float xUnitBase =
            coordinateSpace == GroupCoordinateSpace::PixelSpace ? percentBaseWidth : 100.0f;
        const float yUnitBase =
            coordinateSpace == GroupCoordinateSpace::PixelSpace ? percentBaseHeight : 100.0f;

        std::string groupsContent;
        if (!JsonParseUtils::ExtractArrayContent(json, "groups", groupsContent))
        {
            return groups;
        }

        const auto groupJsonList = JsonParseUtils::ExtractObjectContentsFromArray("[" + groupsContent + "]");
        for (const auto &groupJson : groupJsonList)
        {
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
                adjustment.offsetX = JsonParseUtils::ExtractFloat(groupJson, "offsetXPercent") * xUnitBase / 100.0f;
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
                adjustment.offsetY = JsonParseUtils::ExtractFloat(groupJson, "offsetYPercent") * yUnitBase / 100.0f;
            }
            else if (JsonParseUtils::HasKey(groupJson, "offsetY"))
            {
                adjustment.offsetY = JsonParseUtils::ExtractFloat(groupJson, "offsetY");
            }
            else if (JsonParseUtils::HasKey(groupJson, "moveY"))
            {
                adjustment.offsetY = JsonParseUtils::ExtractFloat(groupJson, "moveY");
            }

            if (JsonParseUtils::HasKey(groupJson, "scalePivotXPercent") &&
                JsonParseUtils::HasKey(groupJson, "scalePivotYPercent"))
            {
                adjustment.scalePivotX = JsonParseUtils::ExtractFloat(groupJson, "scalePivotXPercent") * xUnitBase / 100.0f;
                adjustment.scalePivotY = JsonParseUtils::ExtractFloat(groupJson, "scalePivotYPercent") * yUnitBase / 100.0f;
                adjustment.hasScalePivot = true;
            }
            else if (JsonParseUtils::HasKey(groupJson, "scalePivotX") &&
                     JsonParseUtils::HasKey(groupJson, "scalePivotY"))
            {
                adjustment.scalePivotX = JsonParseUtils::ExtractFloat(groupJson, "scalePivotX");
                adjustment.scalePivotY = JsonParseUtils::ExtractFloat(groupJson, "scalePivotY");
                adjustment.hasScalePivot = true;
            }

            if (JsonParseUtils::HasKey(groupJson, "scalePosition"))
            {
                adjustment.scalePosition = JsonParseUtils::ExtractBool(groupJson, "scalePosition", adjustment.scalePosition);
            }

            groups[id] = adjustment;
        }

        return groups;
    }
} // namespace LayoutParseUtils

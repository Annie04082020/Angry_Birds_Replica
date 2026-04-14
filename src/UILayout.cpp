#include "UILayout.hpp"

#include "JsonParseUtils.hpp"

namespace UILayout
{
    void ParseSection(const std::string &json,
                      const std::string &section,
                      SectionLayout &layout)
    {
        std::string sectionJson;
        if (!JsonParseUtils::ExtractObjectContent(json, section, sectionJson))
        {
            return;
        }

        layout.xPercent = JsonParseUtils::ExtractFloat(sectionJson, "xPercent", layout.xPercent);
        layout.yPercent = JsonParseUtils::ExtractFloat(sectionJson, "yPercent", layout.yPercent);
        layout.scale = JsonParseUtils::ExtractFloat(sectionJson, "scale", layout.scale);
        layout.groupId = JsonParseUtils::ExtractString(sectionJson, "groupId", layout.groupId);
    }

    void ParseCompositeSection(const std::string &json,
                               const std::string &section,
                               CompositeSectionLayout &layout)
    {
        ParseSection(json, section, layout);

        std::string sectionJson;
        if (!JsonParseUtils::ExtractObjectContent(json, section, sectionJson))
        {
            return;
        }

        layout.baseScale = JsonParseUtils::ExtractFloat(sectionJson, "baseScale", layout.baseScale);
        layout.overlayScale = JsonParseUtils::ExtractFloat(sectionJson, "overlayScale", layout.overlayScale);
        layout.hoverScaleMultiplier =
            JsonParseUtils::ExtractFloat(sectionJson, "hoverScaleMultiplier", layout.hoverScaleMultiplier);
    }

    void ApplyGroupAdjustment(
        SectionLayout &layout,
        const std::unordered_map<std::string, GroupAdjustment> &groups)
    {
        if (layout.groupId.empty())
        {
            return;
        }

        const auto groupIt = groups.find(layout.groupId);
        if (groupIt == groups.end())
        {
            return;
        }

        const GroupAdjustment &group = groupIt->second;

        // Apply offset (fixed displacement, independent of scale)
        layout.xPercent += group.offsetX;
        layout.yPercent += group.offsetY;

        // Apply scale-based position transformation relative to pivot
        if (group.scalePosition && group.hasScalePivot && group.scaleMultiplier != 1.0f)
        {
            float relX = layout.xPercent - group.scalePivotX;
            float relY = layout.yPercent - group.scalePivotY;
            relX *= group.scaleMultiplier;
            relY *= group.scaleMultiplier;
            layout.xPercent = group.scalePivotX + relX;
            layout.yPercent = group.scalePivotY + relY;
        }

        layout.scale *= group.scaleMultiplier;
    }

    void ApplyResolutionScaling(
        SectionLayout &layout,
        const glm::vec2 &actualViewportSize,
        float designViewportWidth,
        float designViewportHeight)
    {
        // Calculate scaling factors for each axis
        const float scaleFactorX = actualViewportSize.x / designViewportWidth;
        const float scaleFactorY = actualViewportSize.y / designViewportHeight;

        // Use the average scale factor to maintain consistent proportions
        const float scaleFactor = (scaleFactorX + scaleFactorY) / 2.0f;

        // Apply scaling to the section's scale value
        layout.scale *= scaleFactor;
    }

    glm::vec2 PercentToWorldPosition(float xPercent,
                                     float yPercent,
                                     const glm::vec2 &viewportSize)
    {
        const float x = (xPercent / 100.0f - 0.5f) * viewportSize.x;
        const float y = (0.5f - yPercent / 100.0f) * viewportSize.y;
        return {x, y};
    }
} // namespace UILayout

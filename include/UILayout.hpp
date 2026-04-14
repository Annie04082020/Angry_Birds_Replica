#ifndef UI_LAYOUT_HPP
#define UI_LAYOUT_HPP

#include <unordered_map>
#include <string>

#include "LayoutTypes.hpp"
#include "glm/vec2.hpp"

namespace UILayout
{
    struct SectionLayout
    {
        float xPercent = 50.0f;
        float yPercent = 50.0f;
        float scale = 1.0f;
        std::string groupId;
    };

    struct CompositeSectionLayout : SectionLayout
    {
        float baseScale = 1.0f;
        float overlayScale = 1.0f;
        float hoverScaleMultiplier = 1.125f;
    };

    void ParseSection(const std::string &json,
                      const std::string &section,
                      SectionLayout &layout);

    void ParseCompositeSection(const std::string &json,
                               const std::string &section,
                               CompositeSectionLayout &layout);

    void ApplyGroupAdjustment(
        SectionLayout &layout,
        const std::unordered_map<std::string, GroupAdjustment> &groups);

    // Apply resolution scaling to adjust layout for different viewport sizes
    // designViewportWidth and designViewportHeight specify the resolution the layout was designed for
    void ApplyResolutionScaling(
        SectionLayout &layout,
        const glm::vec2 &actualViewportSize,
        float designViewportWidth = 2400.0f,
        float designViewportHeight = 1350.0f);

    glm::vec2 PercentToWorldPosition(float xPercent,
                                     float yPercent,
                                     const glm::vec2 &viewportSize);
} // namespace UILayout

#endif // UI_LAYOUT_HPP

#include "IntroLayout.hpp"

#include "LayoutParseUtils.hpp"
#include "Util/TransformUtils.hpp"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace
{
    std::vector<UILayout::SectionLayout *> CollectSections(IntroLayout &layout)
    {
        return {
            &layout.play,
            &layout.exit,
            &layout.settingButtonBase,
            &layout.settingButtonOverlay,
            &layout.additionalButtonBase,
            &layout.additionalButtonOverlay,
            &layout.exitConfirm,
            &layout.exitYes,
            &layout.exitNo,
            &layout.exitDialog,
        };
    }

    void ApplyResolutionScalingToComposite(
        UILayout::CompositeSectionLayout &layout,
        float scaleFactor)
    {
        // Scale all the scale-related fields
        layout.scale *= scaleFactor;
        layout.baseScale *= scaleFactor;
        layout.overlayScale *= scaleFactor;
    }

    std::unordered_map<std::string, GroupAdjustment> BuildGroupsWithAutoPivot(
        const std::unordered_map<std::string, GroupAdjustment> &groups,
        const std::vector<UILayout::SectionLayout *> &sections)
    {
        std::unordered_map<std::string, GroupAdjustment> resolvedGroups = groups;

        struct PivotAccumulator
        {
            float sumX = 0.0f;
            float sumY = 0.0f;
            int count = 0;
        };

        std::unordered_map<std::string, PivotAccumulator> accumulators;

        for (const auto *section : sections)
        {
            if (!section || section->groupId.empty())
            {
                continue;
            }

            auto groupIt = resolvedGroups.find(section->groupId);
            if (groupIt == resolvedGroups.end())
            {
                continue;
            }

            const GroupAdjustment &group = groupIt->second;
            if (!group.scalePosition || group.hasScalePivot || group.scaleMultiplier == 1.0f)
            {
                continue;
            }

            auto &acc = accumulators[section->groupId];
            acc.sumX += section->xPercent;
            acc.sumY += section->yPercent;
            ++acc.count;
        }

        for (auto &[groupId, acc] : accumulators)
        {
            if (acc.count <= 0)
            {
                continue;
            }

            auto groupIt = resolvedGroups.find(groupId);
            if (groupIt == resolvedGroups.end())
            {
                continue;
            }

            groupIt->second.scalePivotX = acc.sumX / static_cast<float>(acc.count);
            groupIt->second.scalePivotY = acc.sumY / static_cast<float>(acc.count);
            groupIt->second.hasScalePivot = true;
        }

        return resolvedGroups;
    }
}

namespace IntroLayoutLoader
{
    IntroLayout Load(const std::string &layoutPath)
    {
        IntroLayout layout;

        std::ifstream layoutFile(layoutPath);
        if (!layoutFile)
        {
            return layout;
        }

        std::stringstream buffer;
        buffer << layoutFile.rdbuf();
        const std::string layoutJson = buffer.str();

        UILayout::ParseSection(layoutJson, "play", layout.play);
        UILayout::ParseSection(layoutJson, "exit", layout.exit);
        UILayout::ParseCompositeSection(layoutJson, "settingButtonBase", layout.settingButtonBase);
        UILayout::ParseCompositeSection(layoutJson, "settingButtonOverlay", layout.settingButtonOverlay);
        UILayout::ParseCompositeSection(layoutJson, "additionalButtonBase", layout.additionalButtonBase);
        UILayout::ParseCompositeSection(layoutJson, "additionalButtonOverlay", layout.additionalButtonOverlay);
        UILayout::ParseSection(layoutJson, "exitConfirm", layout.exitConfirm);
        UILayout::ParseSection(layoutJson, "exitYes", layout.exitYes);
        UILayout::ParseSection(layoutJson, "exitNo", layout.exitNo);
        UILayout::ParseSection(layoutJson, "exitDialog", layout.exitDialog);

        const auto groups = LayoutParseUtils::ExtractGroupAdjustments(
            layoutJson,
            LayoutParseUtils::GroupCoordinateSpace::PercentSpace);

        auto sections = CollectSections(layout);
        const auto resolvedGroups = BuildGroupsWithAutoPivot(groups, sections);
        for (auto *section : sections)
        {
            UILayout::ApplyGroupAdjustment(*section, resolvedGroups);
        }

        // Apply resolution scaling to adapt layout for different viewport sizes
        // Design viewport: 2400x1350. Calculate scale factor based on actual viewport.
        const glm::vec2 actualViewportSize = Util::GetViewportSize();
        const float scaleFactorX = actualViewportSize.x / 2400.0f;
        const float scaleFactorY = actualViewportSize.y / 1350.0f;
        const float scaleFactor = (scaleFactorX + scaleFactorY) / 2.0f;

        // Apply resolution scaling to all sections
        UILayout::ApplyResolutionScaling(layout.play, actualViewportSize);
        UILayout::ApplyResolutionScaling(layout.exit, actualViewportSize);
        UILayout::ApplyResolutionScaling(layout.exitConfirm, actualViewportSize);
        UILayout::ApplyResolutionScaling(layout.exitYes, actualViewportSize);
        UILayout::ApplyResolutionScaling(layout.exitNo, actualViewportSize);
        UILayout::ApplyResolutionScaling(layout.exitDialog, actualViewportSize);

        // For composite sections, also scale baseScale and overlayScale
        ApplyResolutionScalingToComposite(layout.settingButtonBase, scaleFactor);
        ApplyResolutionScalingToComposite(layout.settingButtonOverlay, scaleFactor);
        ApplyResolutionScalingToComposite(layout.additionalButtonBase, scaleFactor);
        ApplyResolutionScalingToComposite(layout.additionalButtonOverlay, scaleFactor);

        return layout;
    }
} // namespace IntroLayoutLoader

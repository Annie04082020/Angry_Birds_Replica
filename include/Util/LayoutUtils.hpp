// Layout-related helpers (group multipliers, menu item layout)
#pragma once

#include "IntroLayout.hpp"
#include "Util/GameObject.hpp"
#include <string>

namespace Util
{
    namespace LayoutUtils
    {
        inline float GetGroupScaleMultiplier(const IntroLayout &layout, const std::string &groupId)
        {
            if (groupId.empty())
                return 1.0f;
            auto it = layout.groups.find(groupId);
            if (it != layout.groups.end())
                return it->second.scaleMultiplier;
            return 1.0f;
        }

        inline void ApplyMenuItemLayout(const std::shared_ptr<Util::GameObject> &menuItem,
                                        const std::unordered_map<std::string, MenuItemConfig> &menuItems,
                                        const std::string &itemId,
                                        const glm::vec2 &basePosition,
                                        const IntroLayout &layout)
        {
            const auto itemIt = menuItems.find(itemId);
            if (itemIt == menuItems.end())
            {
                menuItem->m_Transform.translation = basePosition;
                menuItem->m_Transform.scale = {1.0f, 1.0f};
                return;
            }

            const auto &item = itemIt->second;
            const float groupScale = GetGroupScaleMultiplier(layout, item.groupId);
            menuItem->m_Transform.translation = basePosition + glm::vec2{0.0f, item.relativeOffsetY * groupScale};
            menuItem->m_Transform.scale = {item.scale * groupScale, item.scale * groupScale};
        }

    } // namespace LayoutUtils
} // namespace Util

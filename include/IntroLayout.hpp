#ifndef INTRO_LAYOUT_HPP
#define INTRO_LAYOUT_HPP

#include <string>
#include <unordered_map>

#include "LayoutTypes.hpp"
#include "UILayout.hpp"

struct MenuItemConfig
{
    float relativeOffsetY = 0.0f;
    float scale = 1.0f;
    std::string groupId = ""; // Which group this menu item belongs to
};

struct MenuConfig
{
    float itemSpacing = 70.0f;
    float initialOffset = -5.0f;
    float animationDistance = 300.0f;
    float animationSpeed = 400.0f;
    std::unordered_map<std::string, MenuItemConfig> items;
};

struct IntroLayout
{
    MenuConfig menuConfig;
    MenuConfig settingMenuItems;
    MenuConfig additionalMenuItems;

    UILayout::SectionLayout play;
    UILayout::SectionLayout exit;
    UILayout::CompositeSectionLayout settingButtonBase;
    UILayout::CompositeSectionLayout settingButtonOverlay;
    UILayout::CompositeSectionLayout additionalButtonBase;
    UILayout::CompositeSectionLayout additionalButtonOverlay;
    UILayout::SectionLayout exitConfirm;
    UILayout::SectionLayout exitYes;
    UILayout::SectionLayout exitNo;
    UILayout::SectionLayout exitDialog;

    // Store resolved group configurations so menu items can access scale multipliers
    std::unordered_map<std::string, GroupAdjustment> groups;
};

namespace IntroLayoutLoader
{
    IntroLayout Load(const std::string &layoutPath);
}

#endif // INTRO_LAYOUT_HPP

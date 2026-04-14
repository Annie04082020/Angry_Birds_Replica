#ifndef INTRO_LAYOUT_HPP
#define INTRO_LAYOUT_HPP

#include <string>

#include "UILayout.hpp"

struct IntroLayout
{
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
};

namespace IntroLayoutLoader
{
    IntroLayout Load(const std::string &layoutPath);
}

#endif // INTRO_LAYOUT_HPP

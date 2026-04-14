#ifndef LAYOUT_PARSE_UTILS_HPP
#define LAYOUT_PARSE_UTILS_HPP

#include <string>
#include <unordered_map>

#include "LayoutTypes.hpp"

namespace LayoutParseUtils
{
    enum class GroupCoordinateSpace
    {
        PixelSpace,
        PercentSpace,
    };

    std::unordered_map<std::string, std::string> ExtractStringMapField(
        const std::string &json,
        const std::string &fieldKey);

    std::unordered_map<std::string, GroupAdjustment> ExtractGroupAdjustments(
        const std::string &json,
        GroupCoordinateSpace coordinateSpace,
        float percentBaseWidth = 100.0f,
        float percentBaseHeight = 100.0f);
} // namespace LayoutParseUtils

#endif // LAYOUT_PARSE_UTILS_HPP

#ifndef LAYOUT_TYPES_HPP
#define LAYOUT_TYPES_HPP

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

#endif // LAYOUT_TYPES_HPP
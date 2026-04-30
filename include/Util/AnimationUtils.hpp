// Utility helpers for simple UI animations (header-only)
#pragma once

#include "Util/Time.hpp"
#include <cmath>

namespace Util
{
    namespace AnimationUtils
    {
        // Step rotation towards target. Returns true if still animating after the step.
        inline bool StepRotateTowards(float &rotation, float target, float deltaTimeSec, float rotationSpeedRadPerSec)
        {
            float rotationStep = rotationSpeedRadPerSec * deltaTimeSec;
            float remaining = target - rotation;
            if (std::fabs(remaining) <= rotationStep)
            {
                rotation = target;
                return false; // finished
            }
            rotation += (remaining > 0 ? rotationStep : -rotationStep);
            return true; // still animating
        }

    } // namespace AnimationUtils
} // namespace Util

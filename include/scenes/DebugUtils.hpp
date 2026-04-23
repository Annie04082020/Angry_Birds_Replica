#ifndef SCENES_DEBUG_UTILS_HPP
#define SCENES_DEBUG_UTILS_HPP

#include <iostream>
#include <glm/glm.hpp>

namespace DebugUtils
{
    // Toggle logging for collision/sleep debug
    inline constexpr bool kEnableCollisionLog = true;

    inline void LogSleepDecision(const std::string &id, const glm::vec2 &pos, const glm::vec2 &vel, float ang, const char *reason)
    {
        if (!kEnableCollisionLog)
            return;
        try
        {
            std::cout << "[SleepDecision] " << id << " at pos(" << pos.x << "," << pos.y << ") "
                      << "vel(" << vel.x << "," << vel.y << ") ang=" << ang
                      << " reason=" << reason << std::endl;
        }
        catch (...)
        {
        }
    }
}

#endif // SCENES_DEBUG_UTILS_HPP

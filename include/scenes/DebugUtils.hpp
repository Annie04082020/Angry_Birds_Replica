#ifndef SCENES_DEBUG_UTILS_HPP
#define SCENES_DEBUG_UTILS_HPP

#include <glm/glm.hpp>
#include <string>

#include "Util/Logger.hpp"

namespace DebugUtils
{
    // Toggle logging for collision/sleep debug
    inline constexpr bool kEnableCollisionLog = false;

    inline void LogSleepDecision(const std::string &id, const glm::vec2 &pos, const glm::vec2 &vel, float ang, const char *reason)
    {
        if (!kEnableCollisionLog)
            return;
        try
        {
            LOG_DEBUG("SleepDecision {} at pos({},{}) vel({},{}) ang={} reason={}",
                      id,
                      pos.x,
                      pos.y,
                      vel.x,
                      vel.y,
                      ang,
                      reason);
        }
        catch (...)
        {
        }
    }
}

#endif // SCENES_DEBUG_UTILS_HPP

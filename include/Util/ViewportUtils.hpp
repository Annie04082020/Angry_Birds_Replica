#pragma once

#include <glm/glm.hpp>

namespace Util
{
    glm::vec2 GetViewportSize();
    void SetCameraPosition(const glm::vec2 &position);
    glm::vec2 GetCameraPosition();
    void SetCameraZoom(float zoom);
    float GetCameraZoom();
} // namespace Util

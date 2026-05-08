#pragma once

#include "Core/Context.hpp"
#include <glm/glm.hpp>
#include <algorithm>

namespace Util
{
    inline glm::vec2 GetViewportSize()
    {
        const auto context = Core::Context::GetInstance();
        if (!context)
        {
            return {0.0f, 0.0f};
        }

        return {
            static_cast<float>(context->GetWindowWidth()),
            static_cast<float>(context->GetWindowHeight()),
        };
    }

    inline glm::vec2 &CameraPositionStorage()
    {
        static glm::vec2 cameraPosition{0.0f, 0.0f};
        return cameraPosition;
    }

    inline float &CameraZoomStorage()
    {
        static float cameraZoom = 1.0f;
        return cameraZoom;
    }

    inline void SetCameraPosition(const glm::vec2 &position)
    {
        CameraPositionStorage() = position;
    }

    inline glm::vec2 GetCameraPosition()
    {
        return CameraPositionStorage();
    }

    inline void SetCameraZoom(float zoom)
    {
        CameraZoomStorage() = std::clamp(zoom, 0.1f, 4.0f);
    }

    inline float GetCameraZoom()
    {
        return CameraZoomStorage();
    }
} // namespace Util
